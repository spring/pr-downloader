/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Sdp.h"
#include "RapidDownloader.h"
#include "Util.h"
#include "Logger.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileData.h"
#include "FileSystem/HashMD5.h"
#include "FileSystem/AtomicFile.h"
#include "Downloader/CurlWrapper.h"
#include "Downloader/Download.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>

CSdp::CSdp(const std::string& shortname, const std::string& md5, const std::string& name, const std::string& depends, const std::string& url):
	m_download(NULL),
	downloadInitialized(false),
	globalFiles(NULL),
	file_handle(NULL),
	file_pos(0),
	skipped(false),
	cursize(0),
	name(name),
	md5(md5),
	shortname(shortname),
	url(url),
	depends(depends),
	downloaded(false)
{
	memset(this->cursize_buf,0, LENGTH_SIZE);
}

CSdp::~CSdp()
{
	if (file_handle!=NULL) {
		delete file_handle;
	}
}

bool createPoolDirs(const std::string& root)
{
	char buf[1024];
	const int pos = snprintf(buf, sizeof(buf), "%s", root.c_str());
	for (int i=0; i<256; i++) {
		snprintf( buf + pos, 4, "%02x%c", i, PATH_DELIMITER);
		std::string tmp(buf, pos+3);
		if ((!fileSystem->directoryExists(tmp)) && (!fileSystem->createSubdirs(tmp))) {
			LOG_ERROR("Couldn't create %s", tmp.c_str());
			return false;
		}
	}
	return true;
}


bool CSdp::download(IDownload * download)
{
	if (downloaded) //allow download only once of the same sdp
		return true;
	m_download = download;
	filename=fileSystem->getSpringDir() + PATH_DELIMITER+"packages"+PATH_DELIMITER;
	LOG_DEBUG("%s",filename.c_str());
	if (!fileSystem->directoryExists(filename)) {
		fileSystem->createSubdirs(filename);
	}
	int count=0;
	filename  += this->md5 + ".sdp";
	const std::string tmpFile = filename + ".tmp";
	std::list<FileData*> files;

	bool rename = false;
	if (!fileSystem->fileExists(filename)) { //.sdp isn't avaiable, download it
		IDownload dl(tmpFile);
		dl.addMirror(url + "/packages/" + this->md5 + ".sdp");
		httpDownload->download(&dl);
		fileSystem->parseSdp(tmpFile,files); //parse downloaded file
		rename = true;
	} else {
		fileSystem->parseSdp(filename,files); //parse downloaded file
	}

	HashMD5 md5= HashMD5();
	FileData tmp=FileData();
	int i=0;
	for(FileData* filedata: files) { //check which file are available on local disk -> create list of files to download
		i++;
		md5.Set(filedata->md5, sizeof(filedata->md5));
		std::string file;
		fileSystem->getPoolFilename(md5.toString(), file);
		if (!fileSystem->fileExists(file)) { //add non-existing files to download list
			count++;
			filedata->download=true;
		} else {
			filedata->download=false;
		}
		if (i%30==0) {
			LOG_DEBUG("\r%d/%d checked",i,(int)files.size());
		}
	}
	LOG_DEBUG("\r%d/%d need to download %d files",i,(unsigned int)files.size(),count);

	std::string root = fileSystem->getSpringDir();
	root += PATH_DELIMITER;
	root += "pool";
	root += PATH_DELIMITER;
	if (!createPoolDirs(root)) {
		LOG_ERROR("Creating pool directories failed");
		count = 0;
	}
	if (count>0) {
		downloaded=downloadStream(this->url+"/streamer.cgi?"+this->md5,files);
		LOG_DEBUG("Sucessfully downloaded %d files: %s %s",count,shortname.c_str(),name.c_str());
	} else {
		LOG_DEBUG("Already downloaded: %s", shortname.c_str());
		downloaded=true;
	}
	for(FileData* filedata: files) { //free memory
		delete filedata;
	}
	if ((rename) && (!fileSystem->Rename(tmpFile, filename))) {
		LOG_ERROR("Couldn't rename %s to %s", tmpFile.c_str(), filename.c_str());
	}
	return downloaded;
}

/**
	write the data received from curl to the rapid pool.

	the filename is read from the sdp-list (created at request start)
	filesize is read from the http-data received (could overlap!)
*/
static size_t write_streamed_data(const void* tmp, size_t size, size_t nmemb,CSdp *sdp)
{
	char buf[CURL_MAX_WRITE_SIZE];
	memcpy(&buf,tmp,CURL_MAX_WRITE_SIZE);
	if (!sdp->downloadInitialized) {
		sdp->list_it=sdp->globalFiles->begin();
		sdp->downloadInitialized=true;
		sdp->file_handle=NULL;
		sdp->file_name="";
		sdp->skipped=0;
	}
	char* buf_start=(char*)&buf;
	const char* buf_end=buf_start + size*nmemb;
	char* buf_pos=buf_start;

	while (buf_pos<buf_end) { //all bytes written?
		if (sdp->file_handle==NULL) { //no open file, create one
			while ( (!(*sdp->list_it)->download==true) && (sdp->list_it!=sdp->globalFiles->end())) { //get file
				sdp->list_it++;
			}
			HashMD5 md5;
			md5.Set((*sdp->list_it)->md5, sizeof((*sdp->list_it)->md5));
			fileSystem->getPoolFilename(md5.toString(), sdp->file_name);
			sdp->file_handle=new AtomicFile(sdp->file_name);
			if (sdp->file_handle==NULL) {
				LOG_ERROR("couldn't open %s",(*sdp->list_it)->name.c_str());
				return -1;
			}
			sdp->file_pos=0;
		}
		assert(sdp->file_handle!=NULL);
		if (sdp->skipped<LENGTH_SIZE) { // check if we skipped all 4 bytes for file length, if not so, skip them
			const int toskip = intmin(buf_end-buf_pos,LENGTH_SIZE-sdp->skipped); //calculate bytes we can skip, could overlap received bufs
			for (int i=0; i<toskip; i++) { //copy bufs avaiable
				sdp->cursize_buf[sdp->skipped+i]=buf_pos[i];
			}
			sdp->skipped += toskip;
			buf_pos += toskip;
			if (sdp->skipped==LENGTH_SIZE) { // all length bytes read, parse
				(*sdp->list_it)->compsize=parse_int32(sdp->cursize_buf);
				assert((*sdp->list_it)->size+2000 >= (*sdp->list_it)->compsize);
			}
		}
		if (sdp->skipped==LENGTH_SIZE) { // length bytes read
			const int towrite=intmin ((*sdp->list_it)->compsize-sdp->file_pos ,  //minimum of bytes to write left in file and bytes to write left in buf
									  buf_end-buf_pos);
			if (towrite<0) {
				LOG_ERROR("Fatal, something went wrong here! %d, %d %d", towrite, buf_end, buf_pos);
				return -1;
			}
			const int res=sdp->file_handle->Write(buf_pos,towrite);
			if (res!=towrite) {
				LOG_ERROR("fwrite error");
				return -1;
			}
			buf_pos += res;
			sdp->file_pos += res;

			if (sdp->file_pos>=(*sdp->list_it)->compsize) { //file finished -> next file
				sdp->file_handle->Close();
				delete sdp->file_handle;
				sdp->file_handle = NULL;
				if (!fileSystem->fileIsValid(*sdp->list_it,sdp->file_name.c_str())) {
					LOG_ERROR("File is broken?!: %s",sdp->file_name.c_str());
					fileSystem->removeFile(sdp->file_name.c_str());
					return -1;
				}
				sdp->list_it++;
				sdp->file_pos=0;
				sdp->skipped=0;
			}
		}
	}
	return buf_pos-buf_start;

}

/** *
	draw a nice download status-bar
*/
static int progress_func(CSdp& csdp, double TotalToDownload, double NowDownloaded,
						 double TotalToUpload, double NowUploaded)
{

	(void)csdp;
	(void)TotalToUpload;
	(void)NowUploaded; //remove unused warning
	csdp.m_download->rapid_size[&csdp] = TotalToDownload;
	csdp.m_download->map_rapid_progress[&csdp] = NowDownloaded;
	uint64_t total = 0;
	for ( auto it : csdp.m_download->rapid_size) {
		total += it.second;
	}
	csdp.m_download->size = total;
	if (IDownloader::listener != nullptr) {
		IDownloader::listener(NowDownloaded, TotalToDownload);
	}
	total = 0;
	for ( auto it : csdp.m_download->map_rapid_progress) {
		total += it.second;
	}
	csdp.m_download->progress = total;
	if (TotalToDownload == NowDownloaded) //force output when download is finished
		LOG_PROGRESS(NowDownloaded,TotalToDownload, true);
	else
		LOG_PROGRESS(NowDownloaded,TotalToDownload);
	return 0;
}

bool CSdp::downloadStream(const std::string& url,std::list<FileData*> files)
{
	CurlWrapper* curlw = new CurlWrapper();
	if (!curlw) {
		return false;
	}
	CURLcode res;
	LOG_INFO("Using rapid");
	LOG_INFO(url.c_str());

	curl_easy_setopt(curlw->GetHandle(), CURLOPT_URL, url.c_str());

	int  buflen=files.size()/8;
	if (files.size()%8!=0)
		buflen++;
	char* buf=(char*)malloc(buflen); //FIXME: compress blockwise and not all at once
	memset(buf,0,buflen);
	int destlen=files.size()*2;
	LOG_DEBUG("%d %d %d",(int)files.size(),buflen,destlen);
	int i=0;
	for (FileData* it: files) {
		if (it->download==true) {
			buf[i/8] |= (1<<(i%8));
		}
		i++;
	}
	char* dest=(char*)malloc(destlen);

	gzip_str(buf,buflen,dest,&destlen);

	curl_easy_setopt(curlw->GetHandle(), CURLOPT_WRITEFUNCTION, write_streamed_data);
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_WRITEDATA, this);


	globalFiles=&files;
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_POSTFIELDS, dest);
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_POSTFIELDSIZE,destlen);
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_PROGRESSFUNCTION, progress_func);
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_PROGRESSDATA, this);

	res = curl_easy_perform(curlw->GetHandle());
	free(dest);
	/* always cleanup */
	delete curlw;
	curlw = NULL;
	if (res != CURLE_OK) {
		LOG_ERROR("Curl cleanup error: %s",curl_easy_strerror(res));
		return false;
	}
	return true;
}
