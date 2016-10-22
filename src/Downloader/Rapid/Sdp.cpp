/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

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
#include <string>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>

CSdp::CSdp(const std::string& shortname, const std::string& md5,
	   const std::string& name, const std::string& depends,
	   const std::string& baseUrl)
    : m_download(NULL)
    , downloadInitialized(false)
    , file_handle(NULL)
    , file_pos(0)
    , skipped(false)
    , cursize(0)
    , name(name)
    , md5(md5)
    , shortname(shortname)
    , baseUrl(baseUrl)
    , depends(depends)
    , downloaded(false)
{
	memset(cursize_buf, 0, LENGTH_SIZE);
	std::string dir =
	    fileSystem->getSpringDir() + PATH_DELIMITER + "packages" + PATH_DELIMITER;
	LOG_DEBUG("%s", dir.c_str());
	if (!fileSystem->directoryExists(dir)) {
		fileSystem->createSubdirs(dir);
	}
	sdpPath = dir + md5 + ".sdp";
}

CSdp::~CSdp()
{
	if (file_handle != NULL) {
		delete file_handle;
	}
}

bool createPoolDirs(const std::string& root)
{
	char buf[1024];
	const int pos = snprintf(buf, sizeof(buf), "%s", root.c_str());
	for (int i = 0; i < 256; i++) {
		snprintf(buf + pos, 4, "%02x%c", i, PATH_DELIMITER);
		std::string tmp(buf, pos + 3);
		if ((!fileSystem->directoryExists(tmp)) &&
		    (!fileSystem->createSubdirs(tmp))) {
			LOG_ERROR("Couldn't create %s", tmp.c_str());
			return false;
		}
	}
	return true;
}

bool CSdp::downloadSelf(IDownload* dl)
{
	if (!fileSystem->fileExists(sdpPath)) { //.sdp isn't avaiable, download it
		const std::string tmpFile = sdpPath + ".tmp";
		IDownload dl(tmpFile);
		dl.addMirror(baseUrl + "/packages/" + md5 + ".sdp");
		if(!httpDownload->download(&dl)) {
			LOG_ERROR("Couldn't download %s", (md5 + ".sdp").c_str());
			return false;
		}

		if (!fileSystem->Rename(tmpFile, sdpPath)) {
			LOG_ERROR("Couldn't rename %s to %s", tmpFile.c_str(), sdpPath.c_str());
			return false;
		}
	}

	return true;
}

bool CSdp::download(IDownload* dl)
{
	if (downloaded) // allow download only once of the same sdp
		return true;
	m_download = dl;

	if (!downloadSelf(dl))
		return false;

	std::list<FileData> tmpFiles;
	files = &tmpFiles;

	if (!fileSystem->parseSdp(sdpPath, *files))// parse downloaded file
		return false;

	int i = 0;
	int count = 0;
	for (FileData& filedata: *files) { // check which file are available on local
	                                   // disk -> create list of files to download
		HashMD5 fileMd5;
		i++;
		fileMd5.Set(filedata.md5, sizeof(filedata.md5));
		std::string file;
		fileSystem->getPoolFilename(fileMd5.toString(), file);
		if (!fileSystem->fileExists(
			file)) { // add non-existing files to download list
			count++;
			filedata.download = true;
		} else {
			filedata.download = false;
		}
		if (i % 30 == 0) {
			LOG_DEBUG("\r%d/%d checked", i, (int)files->size());
		}
	}
	LOG_DEBUG("\r%d/%d need to download %d files", i, (int)files->size(),
		  count);

	std::string root = fileSystem->getSpringDir();
	root += PATH_DELIMITER;
	root += "pool";
	root += PATH_DELIMITER;
	if (!createPoolDirs(root)) {
		LOG_ERROR("Creating pool directories failed");
		count = 0;
	}
	if (count > 0) {
		downloaded = downloadStream();
		if (!downloaded) {
			LOG_ERROR("Couldn't download files for %s", md5.c_str());
			fileSystem->removeFile(sdpPath);
			return false;
		}
		LOG_DEBUG("Sucessfully downloaded %d files: %s %s", count,
			  shortname.c_str(), name.c_str());
	} else {
		LOG_DEBUG("Already downloaded: %s", shortname.c_str());
		downloaded = true;
	}

	if (downloaded) {
		dl->state = IDownload::STATE_FINISHED;
	}
	return downloaded;
}

/**
        write the data received from curl to the rapid pool.

        the filename is read from the sdp-list (created at request start)
        filesize is read from the http-data received (could overlap!)
*/
static size_t write_streamed_data(const void* tmp, size_t size, size_t nmemb,
				  CSdp* sdp)
{
	if (IDownloader::AbortDownloads())
		return -1;
	char buf[CURL_MAX_WRITE_SIZE];
	memcpy(&buf, tmp, CURL_MAX_WRITE_SIZE);
	if (!sdp->downloadInitialized) {
		sdp->list_it = sdp->files->begin();
		sdp->downloadInitialized = true;
		sdp->file_handle = NULL;
		sdp->file_name = "";
		sdp->skipped = 0;
	}
	char* buf_start = (char*)&buf;
	const char* buf_end = buf_start + size * nmemb;
	char* buf_pos = buf_start;

	while (buf_pos < buf_end) {		// all bytes written?
		if (sdp->file_handle == NULL) { // no open file, create one
			while (!sdp->list_it->download) { // get file
				++sdp->list_it;
			}
			assert(sdp->list_it != sdp->files->end());

			FileData& fd = *(sdp->list_it);
			HashMD5 fileMd5;

			fileMd5.Set(fd.md5, sizeof(fd.md5));
			fileSystem->getPoolFilename(fileMd5.toString(), sdp->file_name);
			sdp->file_handle = new CFile();
			if (sdp->file_handle == NULL) {
				LOG_ERROR("couldn't open %s", fd.name.c_str());
				return -1;
			}
			sdp->file_handle->Open(sdp->file_name);
			sdp->file_pos = 0;
		}
		assert(sdp->file_handle != NULL);
		FileData& fd = *(sdp->list_it);
		if (sdp->skipped < LENGTH_SIZE) { // check if we skipped all 4 bytes for
						  // file length, if not so, skip them
			const int toskip =
			    intmin(buf_end - buf_pos,
				   LENGTH_SIZE - sdp->skipped); // calculate bytes we can skip,
								// could overlap received bufs
			for (int i = 0; i < toskip; i++) {      // copy bufs avaiable
				sdp->cursize_buf[sdp->skipped + i] = buf_pos[i];
			}
			sdp->skipped += toskip;
			buf_pos += toskip;
			if (sdp->skipped == LENGTH_SIZE) { // all length bytes read, parse
				fd.compsize = parse_int32(sdp->cursize_buf);
				assert(fd.size + 2000 >= fd.compsize);
			}
		}
		if (sdp->skipped == LENGTH_SIZE) { // length bytes read
			const int towrite =
			    intmin(fd.compsize -
				       sdp->file_pos, // minimum of bytes to write left in file
						      // and bytes to write left in buf
				   buf_end - buf_pos);
			assert(towrite >= 0);

			if (towrite == 0)
				break;

			const int res = sdp->file_handle->Write(buf_pos, towrite);
			if (res != towrite) {
				LOG_ERROR("fwrite error");
				return -1;
			}
			buf_pos += res;
			sdp->file_pos += res;

			if (sdp->file_pos >=
			    fd.compsize) { // file finished -> next file
				sdp->file_handle->Close();
				delete sdp->file_handle;
				sdp->file_handle = NULL;
				if (!fileSystem->fileIsValid(&fd, sdp->file_name.c_str())) {
					LOG_ERROR("File is broken?!: %s", sdp->file_name.c_str());
					fileSystem->removeFile(sdp->file_name.c_str());
					return -1;
				}
				++sdp->list_it;
				sdp->file_pos = 0;
				sdp->skipped = 0;
			}
		}
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
	(void)sdp;
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

	const int buflen = (files->size() + 7) / 8;
	std::vector<char> buf(buflen, 0);

	int i = 0;
	for (FileData& fd: *files) {
		if (fd.download) {
			buf[i / 8] |= (1 << (i % 8));
		}
		i++;
	}

	int destlen = files->size() * 2;
	std::vector<char> dest(destlen, 0);
	LOG_DEBUG("%d %d %d", (int)files->size(), buflen, destlen);

	gzip_str(&buf[0], buflen, &dest[0], &destlen);

	curl_easy_setopt(curlw.GetHandle(), CURLOPT_WRITEFUNCTION,
			 write_streamed_data);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_WRITEDATA, this);

	curl_easy_setopt(curlw.GetHandle(), CURLOPT_POSTFIELDS, &dest[0]);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_POSTFIELDSIZE, destlen);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_PROGRESSFUNCTION, progress_func);
	curl_easy_setopt(curlw.GetHandle(), CURLOPT_PROGRESSDATA, this);

	res = curl_easy_perform(curlw.GetHandle());
	/* always cleanup */
	if (res != CURLE_OK) {
		LOG_ERROR("Curl error: %s", curl_easy_strerror(res));
		return false;
	}
	return true;
}
