/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <string>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <errno.h>

#include "Sdp.h"
#include "RapidDownloader.h"
#include "Util.h"
#include "Logger.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileData.h"
#include "FileSystem/HashMD5.h"
#include "FileSystem/File.h"
#include "Downloader/CurlWrapper.h"
#include "Downloader/Download.h"

CSdp::CSdp(const std::string& shortname, const std::string& md5,
	   const std::string& name, const std::string& depends,
	   const std::string& baseUrl)
    : name(name)
    , md5(md5)
    , shortname(shortname)
    , baseUrl(baseUrl)
    , depends(depends)
{
	memset(cursize_buf, 0, LENGTH_SIZE);
	const std::string dir =
	    fileSystem->getSpringDir() + PATH_DELIMITER + "packages" + PATH_DELIMITER;
	if (!fileSystem->directoryExists(dir)) {
		fileSystem->createSubdirs(dir);
	}
	sdpPath = dir + md5 + ".sdp";
	LOG_DEBUG("%s", sdpPath.c_str());
}

CSdp::CSdp(CSdp&& sdp) = default;

CSdp::~CSdp() = default;

bool createPoolDirs(const std::string& root)
{
	for (int i = 0; i < 256; i++) {
		char buf[1024];
		const int len = snprintf(buf, sizeof(buf), "%s%02x%c", root.c_str(), i, PATH_DELIMITER);
		const std::string tmp(buf, len);
		if ((!fileSystem->directoryExists(tmp)) &&
		    (!fileSystem->createSubdirs(tmp))) {
			LOG_ERROR("Couldn't create %s", tmp.c_str());
			return false;
		}
	}
	return true;
}

bool CSdp::downloadSelf(IDownload* /*dl*/)
{
	const std::string tmpFile = sdpPath + ".tmp";
	IDownload tmpdl(tmpFile);
	tmpdl.addMirror(baseUrl + "/packages/" + md5 + ".sdp");
	if(!httpDownload->download(&tmpdl)) {
		LOG_ERROR("Couldn't download %s", (md5 + ".sdp").c_str());
		return false;
	}


	if (!fileSystem->Rename(tmpFile, sdpPath)) {
		LOG_ERROR("Couldn't rename %s to %s: %s", tmpFile.c_str(), sdpPath.c_str(), strerror(errno));
		return false;
	}

	return true;
}

bool CSdp::download(IDownload* dl)
{
	if (downloaded) // allow download only once of the same sdp
		return true;
	m_download = dl;
	if ((!fileSystem->fileExists(sdpPath)) || (!fileSystem->parseSdp(sdpPath, files))) {// parse downloaded file
		if (!downloadSelf(dl))
			return false;
		fileSystem->parseSdp(sdpPath, files);
	}

	int i = 0;
	int count = 0;
	for (FileData& filedata: files) { // check which file are available on local
	                                   // disk -> create list of files to download
		HashMD5 fileMd5;
		i++;
		fileMd5.Set(filedata.md5, sizeof(filedata.md5));
		const std::string file = fileSystem->getPoolFilename(fileMd5.toString());
		if (!fileSystem->fileExists(file)) { // add non-existing files to download list
			count++;
			filedata.download = true;
		} else {
			filedata.download = false;
		}
		if (i % 30 == 0) {
			LOG_DEBUG("%d/%d checked", i, (int)files.size());
		}
	}
	LOG_DEBUG("%d/%d need to download %d files", i, (int)files.size(),
		  count);

	const std::string root = fileSystem->getSpringDir() + PATH_DELIMITER + "pool" + PATH_DELIMITER;
	if (!createPoolDirs(root)) {
		LOG_ERROR("Creating pool directories failed");
		return false;
	}
	if (!downloadStream()) {
		LOG_ERROR("Couldn't download files for %s", md5.c_str());
		fileSystem->removeFile(sdpPath);
		return false;
	}
	LOG_DEBUG("Sucessfully downloaded %d files: %s %s", count,
		  shortname.c_str(), name.c_str());

	if (!fileSystem->validateSDP(sdpPath)) { //FIXME: in this call only the downloaded files should be checked
		LOG_ERROR("Validation failed");
		return false;
	}
	downloaded = true;
	dl->state = IDownload::STATE_FINISHED;
	return true;
}

static bool OpenNextFile(CSdp& sdp)
{
	//file already open, return
	if (sdp.file_handle != nullptr) {
		return true;
	}

	// get next file + open it
	while (!sdp.list_it->download) {
		//LOG_ERROR("next file");
		sdp.list_it++;
	}
	assert(sdp.list_it != sdp.files.end());

	HashMD5 fileMd5;
	FileData& fd = *(sdp.list_it);

	fd.compsize = parse_int32(sdp.cursize_buf);
	// LOG_DEBUG("Read length of %d, uncompressed size from sdp: %d", fd.compsize, fd.size);
	assert(fd.size + 5000 >= fd.compsize); // compressed file should be smaller than uncompressed file

	fileMd5.Set(fd.md5, sizeof(fd.md5));
	sdp.file_name = fileSystem->getPoolFilename(fileMd5.toString());
	sdp.file_handle = std::unique_ptr<CFile>(new CFile());
	if (sdp.file_handle == nullptr) {
		LOG_ERROR("couldn't open %s", fd.name.c_str());
		return false;
	}
	sdp.file_handle->Open(sdp.file_name, fd.compsize);
	sdp.file_pos = 0;
	return true;
}

static int GetLength(CSdp& sdp, const char* const buf_pos, const char* const buf_end)
{
	// calculate bytes we can skip, could overlap received bufs
	const int toskip = intmin(buf_end - buf_pos, LENGTH_SIZE - sdp.skipped);
	assert(toskip > 0);
	// copy bufs avaiable
	memcpy(sdp.cursize_buf + sdp.skipped, buf_pos, toskip);
	sdp.skipped += toskip;

//	if (sdp.skipped > 0) { //size was in at least two packets
		LOG_DEBUG("%.2x %.2x %.2x %.2x", sdp.cursize_buf[0], sdp.cursize_buf[1], sdp.cursize_buf[2], sdp.cursize_buf[3]);
//	}

	return toskip;
}

static void SafeCloseFile(CSdp& sdp)
{
	if (sdp.file_handle == nullptr)
		return;

	sdp.file_handle->Close();
	sdp.file_handle = nullptr;
	sdp.file_pos = 0;
	sdp.skipped = 0;
}

static int WriteData(CSdp& sdp, const char* const buf_pos, const char* const buf_end)
{
	// minimum of bytes to write left in file and bytes to write left in buf
	const FileData& fd = *(sdp.list_it);
	const long towrite = intmin(fd.compsize - sdp.file_pos, buf_end - buf_pos);
//	LOG_DEBUG("towrite: %d total size: %d, uncomp size: %d pos: %d", towrite, fd.compsize,fd.size, sdp.file_pos);
	assert(towrite >= 0);
	assert(fd.compsize > 0); //.gz are always > 0

	int res = 0;
	if (towrite > 0) {
		res = sdp.file_handle->Write(buf_pos, towrite);
	}
	if (res > 0) {
		sdp.file_pos += res;
	}
	if (res != towrite) {
		LOG_ERROR("fwrite error");
		return false;
	}

	// file finished -> next file
	if (sdp.file_pos >= fd.compsize) {
		SafeCloseFile(sdp);
		if (!fileSystem->fileIsValid(&fd, sdp.file_name.c_str())) {
			LOG_ERROR("File is broken?!: %s", sdp.file_name.c_str());
			fileSystem->removeFile(sdp.file_name.c_str());
			return -1;
		}
		++sdp.list_it;
		memset(sdp.cursize_buf, 0, 4); //safety
	}
	return res;
}

void dump_data(CSdp& sdp, const char* const /*buf_pos*/, const char* const /*buf_end*/)
{
	LOG_WARN("%s %d\n", sdp.file_name.c_str(), sdp.list_it->compsize);
}


/**
        write the data received from curl to the rapid pool.

        the filename is read from the sdp-list (created at request start)
        filesize is read from the http-data received (could overlap!)
*/
static size_t write_streamed_data(const void* buf, size_t size, size_t nmemb, CSdp* psdp)
{
	//LOG_DEBUG("write_stream_data bytes read: %d", size * nmemb);
	if (psdp == nullptr) {
		LOG_ERROR("nullptr in write_stream_data");
		return -1;
	}
	CSdp& sdp = *psdp;

	if (IDownloader::AbortDownloads())
		return -1;
	const char* buf_start = (const char*)buf;
	const char* buf_end = buf_start + size * nmemb;
	const char* buf_pos = buf_start;

	// all bytes written?
	while (buf_pos < buf_end) {
		// check if we skipped all 4 bytes for
		if (sdp.skipped < LENGTH_SIZE) {
			const int skipped = GetLength(sdp, buf_pos, buf_end);
			buf_pos += skipped;
		}
		if (sdp.skipped < LENGTH_SIZE) {
			LOG_DEBUG("packed end, skipped: %d, bytes left: %d", sdp.skipped, buf_end - buf_pos);
			assert(buf_pos == buf_end);
			break;
		}

		assert(sdp.skipped == LENGTH_SIZE);

		if (!OpenNextFile(sdp))
			return -1;

		assert(sdp.file_handle != nullptr);
		assert(sdp.list_it != sdp.files.end());

		const int written = WriteData(sdp, buf_pos, buf_end);
		if (written < 0) {
			dump_data(sdp, buf_pos, buf_end);
			return -1;
		}
		buf_pos += written;
	}
	return buf_pos - buf_start;
}

/** *
        draw a nice download status-bar
*/
static int progress_func(CSdp& sdp, double TotalToDownload,
			 double NowDownloaded, double TotalToUpload,
			 double NowUploaded)
{
	if (IDownloader::AbortDownloads())
		return -1;
	(void)TotalToUpload;
	(void)NowUploaded; // remove unused warning
	sdp.m_download->rapid_size[&sdp] = TotalToDownload;
	sdp.m_download->map_rapid_progress[&sdp] = NowDownloaded;
	uint64_t total = 0;
	for (auto it : sdp.m_download->rapid_size) {
		total += it.second;
	}
	sdp.m_download->size = total;
	if (IDownloader::listener != nullptr) {
		IDownloader::listener(NowDownloaded, TotalToDownload);
	}
	total = 0;
	for (auto it : sdp.m_download->map_rapid_progress) {
		total += it.second;
	}
	sdp.m_download->progress = total;
	if (TotalToDownload == NowDownloaded) // force output when download is
					      // finished
		LOG_PROGRESS(NowDownloaded, TotalToDownload, true);
	else
		LOG_PROGRESS(NowDownloaded, TotalToDownload);
	return 0;
}

bool CSdp::downloadStream()
{
	std::string downloadUrl = baseUrl + "/streamer.cgi?" + md5;
	CurlWrapper curlw;

	CURLcode res;
	LOG_INFO("Using rapid");
	LOG_INFO(downloadUrl.c_str());

	curl_easy_setopt(curlw.GetHandle(), CURLOPT_URL, downloadUrl.c_str());

	SafeCloseFile(*this);

	list_it = files.begin();
	file_name = "";

	const int buflen = (files.size() / 8) + 1;
	std::vector<char> buf(buflen, 0);

	int i = 0;
	for (FileData& fd: files) {
		if (fd.download) {
			buf[i / 8] |= (1 << (i % 8));
		}
		i++;
	}

	int destlen = files.size() * 2 + 1024;
	std::vector<char> dest(destlen, 0);
	LOG_DEBUG("Files: %d Buflen: %d Destlen: %d", (int)files.size(), buflen, destlen);

	gzip_str(&buf[0], buflen, &dest[0], &destlen);

	curl_easy_setopt(curlw.GetHandle(), CURLOPT_WRITEFUNCTION, write_streamed_data);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_POSTFIELDS, &dest[0]);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_POSTFIELDSIZE, destlen);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_PROGRESSFUNCTION, progress_func);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_PROGRESSDATA, this);

	res = curl_easy_perform(curlw.GetHandle());

	SafeCloseFile(*this);

	/* always cleanup */
	if (res != CURLE_OK) {
		LOG_ERROR("Curl error: %s", curl_easy_strerror(res));
		return false;
	}


	return true;
}
