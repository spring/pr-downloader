/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "HttpDownloader.h"
#include "DownloadData.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/HashMD5.h"
#include "FileSystem/HashSHA1.h"
#include "Util.h"
#include "Logger.h"
#include "Downloader/Mirror.h"
#include "lib/xmlrpc++/src/XmlRpc.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif

#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <stdlib.h>

CHttpDownloader::CHttpDownloader()
{
}

CHttpDownloader::~CHttpDownloader()
{
}

bool CHttpDownloader::search(std::list<IDownload*>& res, const std::string& name, IDownload::category cat)
{
	LOG_DEBUG("%s", name.c_str()  );

	const std::string method(XMLRPC_METHOD);
	//std::string category;
	XmlRpc::XmlRpcClient client(XMLRPC_HOST,XMLRPC_PORT, XMLRPC_URI);
	XmlRpc::XmlRpcValue arg;
	arg["springname"]=name;
	arg["torrent"]=true;
	switch(cat) {
	case IDownload::CAT_MAPS:
		arg["category"]="map";
		break;
	case IDownload::CAT_GAMES:
		arg["category"]="game";
		break;
	case IDownload::CAT_ENGINE_LINUX:
		arg["category"]="engine_linux";
		break;
	case IDownload::CAT_ENGINE_WINDOWS:
		arg["category"]="engine_windows";
		break;
	case IDownload::CAT_ENGINE_MACOSX:
		arg["category"]="engine_macosx";
		break;
	default:
		break;
	}

	XmlRpc::XmlRpcValue result;
	client.execute(method.c_str(),arg, result);


	if (result.getType()!=XmlRpc::XmlRpcValue::TypeArray) {
		return false;
	}

	for(int i=0; i<result.size(); i++) {
		XmlRpc::XmlRpcValue resfile = result[i];

		if (resfile.getType()!=XmlRpc::XmlRpcValue::TypeStruct) {
			return false;
		}
		if (resfile["category"].getType()!=XmlRpc::XmlRpcValue::TypeString) {
			LOG_ERROR("No category in result");
			return false;
		}
		std::string filename=fileSystem->getSpringDir();
		std::string category=resfile["category"];
		filename+=PATH_DELIMITER;
		if (category=="map")
			filename+="maps";
		else if (category=="game")
			filename+="games";
		else if (category.find("engine")==0) // engine_windows, engine_linux, engine_macosx
			filename+="engine";
		else
			LOG_ERROR("Unknown Category %s", category.c_str());
		filename+=PATH_DELIMITER;
		if ((resfile["mirrors"].getType()!=XmlRpc::XmlRpcValue::TypeArray) ||
		    (resfile["filename"].getType()!=XmlRpc::XmlRpcValue::TypeString)) {
			LOG_ERROR("Invalid type in result");
			return false;
		}
		filename.append(resfile["filename"]);
		IDownload* dl=new IDownload(filename);
		XmlRpc::XmlRpcValue mirrors = resfile["mirrors"];
		for(int j=0; j<mirrors.size(); j++) {
			if (mirrors[j].getType()!=XmlRpc::XmlRpcValue::TypeString) {
				LOG_ERROR("Invalid type in result");
			} else {
				dl->addMirror(mirrors[j]);
			}
		}

		if(resfile["torrent"].getType()==XmlRpc::XmlRpcValue::TypeBase64) {
			const std::vector<char> torrent = resfile["torrent"];
			fileSystem->parseTorrent(&torrent[0], torrent.size(), dl);
		}
		if (resfile["version"].getType()==XmlRpc::XmlRpcValue::TypeString) {
			const std::string& version = resfile["version"];
			dl->version = version;
		}
		if (resfile["md5"].getType()==XmlRpc::XmlRpcValue::TypeString) {
			dl->hash=new HashMD5();
			dl->hash->Set(resfile["md5"]);
		}
		if (resfile["size"].getType()==XmlRpc::XmlRpcValue::TypeInt) {
			dl->size=resfile["size"];
		}
		if (resfile["depends"].getType() == XmlRpc::XmlRpcValue::TypeArray) {
			for(int i=0; i<resfile["depends"].size(); i++) {
				if (resfile["depends"][i].getType() == XmlRpc::XmlRpcValue::TypeString) {
					const std::string &dep = resfile["depends"][i];
					dl->addDepend(dep);
				}
			}
		}
		res.push_back(dl);
	}
	return true;
}

size_t multi_write_data(void *ptr, size_t size, size_t nmemb, DownloadData* data)
{
	//LOG_DEBUG("%d %d",size,  nmemb);
	if (!data->headersok) {
		LOG_ERROR("Server returned invalid / to few headers!");
		return -1;
	}
	return data->download->file->Write((const char*)ptr, size*nmemb, data->piece);
}

size_t multiHeader(void *ptr, size_t size, size_t nmemb, DownloadData *data)
{
	if(data->download->pieces.empty()) { //no chunked transfer, don't check headers
		LOG_DEBUG("Unchunked transfer!");
		data->headersok = true;
		return size*nmemb;
	}
	const std::string buf((char*)ptr, size*nmemb-1);
	int start, end, total;
	int count = sscanf(buf.c_str(), "Content-Range: bytes %d-%d/%d", &start, &end, &total);
	if(count == 3) {
		int piecesize = data->download->file->GetPieceSize(data->piece);
		if (end-start+1!=piecesize) {
			LOG_DEBUG("piecesize %d doesn't match server size: %d", piecesize, end-start+1);
			return -1;
		}
		data->headersok = true;
	}
	LOG_DEBUG("%s", buf.c_str());
	return size*nmemb;
}

bool CHttpDownloader::getRange(std::string& range, int piece, int piecesize)
{
	std::ostringstream s;
	s << (int)(piecesize*piece) <<"-"<< (piecesize*piece) + piecesize-1;
	range=s.str();
	LOG_DEBUG("%s", range.c_str());
	return true;
}

void CHttpDownloader::showProcess(IDownload* download)
{
	int done = download->getProgress();
	int size = download->size;
	if ((size<0) && (download->state==IDownload::STATE_FINISHED)) {
		size=done;
		LOG_PROGRESS(done, size, true);
	} else {
		LOG_PROGRESS(done, size);
	}
}

int CHttpDownloader::verifyAndGetNextPiece(CFile& file, IDownload* download)
{
	//verify file by md5 if pieces.size == 0
	if((download->pieces.empty()) && (download->hash!=NULL) && (download->hash->isSet())) {
		HashMD5 md5=HashMD5();
		file.Hash(md5);
		if (md5.compare(download->hash)) {
			LOG_INFO("md5 correct: %s", md5.toString().c_str());
			download->state=IDownload::STATE_FINISHED;
			return -1;
		} else {
			LOG_ERROR("md5 sum missmatch %s %s", download->hash->toString().c_str(), md5.toString().c_str());
		}
	}

	HashSHA1 sha1=HashSHA1();
	unsigned alreadyDl=0;
	for(unsigned i=0; i<download->pieces.size(); i++ ) { //find first not downloaded piece
		showProcess(download);
		if (download->pieces[i].state==IDownload::STATE_FINISHED) {
			alreadyDl++;
			continue;
		} else if (download->pieces[i].state==IDownload::STATE_NONE) {
			if ((download->pieces[i].sha->isSet()) && (!file.IsNewFile())) { //reuse piece, if checksum is fine
				file.Hash(sha1, i);
//	LOG("bla %s %s", sha1.toString().c_str(), download.pieces[i].sha->toString().c_str());
				if (sha1.compare(download->pieces[i].sha)) {
//					LOG_DEBUG("piece %d has already correct checksum, reusing", i);
					download->pieces[i].state=IDownload::STATE_FINISHED;
					alreadyDl++;
					continue;
				}
			}
			return i;
		}
	}
	if (!download->pieces.empty()) {
		download->state=IDownload::STATE_FINISHED;
	}
	return -1;
}

bool CHttpDownloader::setupDownload(DownloadData* piece)
{
	int pieceNum=verifyAndGetNextPiece(*(piece->download->file), piece->download);
	if (piece->download->state==IDownload::STATE_FINISHED)
		return false;
	assert(piece->download->pieces.size()<=0 || pieceNum>=0);

	piece->piece=pieceNum;
	if (piece->easy_handle==NULL) {
		piece->easy_handle=curl_easy_init();
	} else {
		curl_easy_reset(piece->easy_handle);
	}
	curl_easy_setopt(piece->easy_handle, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(piece->easy_handle, CURLOPT_TIMEOUT, 30);
	CURL* curle= piece->easy_handle;
	piece->mirror=piece->download->getFastestMirror();
	if (piece->mirror==NULL) {
		LOG_ERROR("No mirror found");
		return false;
	}
	std::string escaped;
	piece->mirror->escapeUrl(escaped);

	curl_easy_setopt(curle, CURLOPT_WRITEFUNCTION, multi_write_data);
	curl_easy_setopt(curle, CURLOPT_WRITEDATA, piece);
	curl_easy_setopt(curle, CURLOPT_USERAGENT, USER_AGENT);
	curl_easy_setopt(curle, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curle, CURLOPT_URL, escaped.c_str());

	if ((piece->download->size>0) && (pieceNum>=0)) { //don't set range, if size unknown
		std::string range;
		if (!getRange(range, pieceNum, piece->download->piecesize)) {
			LOG_ERROR("Error getting range for download");
			return false;
		}
		//set range for request, format is <start>-<end>
		curl_easy_setopt(curle, CURLOPT_RANGE, range.c_str());
		//parse server response	header as well
		curl_easy_setopt(curle, CURLOPT_HEADERFUNCTION, multiHeader);
		curl_easy_setopt(curle, CURLOPT_WRITEHEADER, piece);
		piece->download->pieces[pieceNum].state=IDownload::STATE_DOWNLOADING;
	} else { //
		LOG_DEBUG("single piece transfer");
		piece->headersok = true;

		//this sets the header If-Modified-Since -> downloads only when remote file is newer than local file
		const long timestamp = piece->download->file->GetTimestamp();
		curl_easy_setopt(curle, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
		curl_easy_setopt(curle, CURLOPT_TIMEVALUE, timestamp );
		curl_easy_setopt(curle, CURLOPT_FILETIME, 1);
	}
	return true;
}

DownloadData* CHttpDownloader::getDataByHandle(const std::vector <DownloadData*>& downloads, const CURL* easy_handle) const
{
	for(int i=0; i<(int)downloads.size(); i++) { //search corresponding data structure
		if (downloads[i]->easy_handle == easy_handle) {
			return downloads[i];
		}
	}
	return NULL;
}

bool CHttpDownloader::processMessages(CURLM* curlm, std::vector <DownloadData*>& downloads)
{
	int msgs_left;
	HashSHA1 sha1;
	bool aborted=false;
	while(struct CURLMsg* msg=curl_multi_info_read(curlm, &msgs_left)) {
		switch(msg->msg) {
		case CURLMSG_DONE: { //a piece has been downloaded, verify it
			DownloadData* data=getDataByHandle(downloads, msg->easy_handle);
			switch(msg->data.result) {
			case CURLE_OK:
				break;
			case CURLE_HTTP_RETURNED_ERROR: //some 4* HTTP-Error (file not found, access denied,...)
			default:
				long http_code = 0;
				curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
				LOG_ERROR("CURL error(%d): %s %d (%s)",msg->msg, curl_easy_strerror(msg->data.result), http_code, data->mirror->url.c_str());
				if (data->piece>=0) {
					data->download->pieces[data->piece].state=IDownload::STATE_NONE;
				}
				data->mirror->status=Mirror::STATUS_BROKEN;
				//FIXME: cleanup curl handle here + process next dl
			}
			if (data==NULL) {
				LOG_ERROR("Couldn't find download in download list");
				return false;
			}
			if (data->piece<0) { //download without pieces
				return false;
			}
			assert(data->download->file!=NULL);
			assert(data->piece< (int)data->download->pieces.size());
			if (data->download->pieces[data->piece].sha->isSet()) {
				data->download->file->Hash(sha1, data->piece);
				if (sha1.compare(data->download->pieces[data->piece].sha)) { //piece valid
					data->download->pieces[data->piece].state=IDownload::STATE_FINISHED;
//					LOG("piece %d verified!", data->piece);
				} else { //piece download broken, mark mirror as broken (for this file)
					data->download->pieces[data->piece].state=IDownload::STATE_NONE;
					data->mirror->status=Mirror::STATUS_BROKEN;
					//FIXME: cleanup curl handle here + process next dl
				}
			} else {
				LOG_INFO("sha1 checksum seems to be not set, can't check received piece %d", data->piece);
			}
			//get speed at which this piece was downloaded + update mirror info
			double dlSpeed;
			curl_easy_getinfo(data->easy_handle, CURLINFO_SPEED_DOWNLOAD, &dlSpeed);
			data->mirror->UpdateSpeed(dlSpeed);
			if (data->mirror->status == Mirror::STATUS_UNKNOWN) //set mirror status only when unset
				data->mirror->status=Mirror::STATUS_OK;

			//remove easy handle, as its finished
			curl_multi_remove_handle(curlm, data->easy_handle);
			curl_easy_cleanup(data->easy_handle);
			data->easy_handle=NULL;

			//piece finished / failed, try a new one
			if (!setupDownload(data)) {
				LOG_DEBUG("No piece found, all pieces finished / currently downloading");
				break;
			}
			int ret=curl_multi_add_handle(curlm, data->easy_handle);
			if (ret!=CURLM_OK) {
				LOG_ERROR("curl_multi_perform_error: %d %d", ret, CURLM_BAD_EASY_HANDLE);
			}
			break;
		}
		default:
			LOG_ERROR("Unhandled message %d", msg->msg);
		}
	}
	return aborted;
}

bool CHttpDownloader::download(std::list<IDownload*>& download)
{

	std::list<IDownload*>::iterator it;
	std::vector <DownloadData*> downloads;
	CURLM* curlm=curl_multi_init();
	for(it=download.begin(); it!=download.end(); ++it) {
		const int count=std::min(MAX_PARALLEL_DOWNLOADS, std::max(1, std::min((int)(*it)->pieces.size(), (*it)->getMirrorCount()))); //count of parallel downloads
		if((*it)->getMirrorCount()<=0) {
			LOG_ERROR("No mirrors found");
			return false;
		}
		LOG_DEBUG("Using %d parallel downloads", count);
		CFile* file=new CFile();
		if(!file->Open((*it)->name, (*it)->size, (*it)->piecesize)) {
			delete file;
			return false;
		}
		(*it)->file = file;
		for(int i=0; i<count; i++) {
			DownloadData* dlData=new DownloadData();
			dlData->download=*it;
			if (!setupDownload(dlData)) { //no piece found (all pieces already downloaded), skip
				delete dlData;
				if ((*it)->state!=IDownload::STATE_FINISHED) {
					LOG_ERROR("no piece found");
					return false;
				}
			} else {
				downloads.push_back(dlData);
				curl_multi_add_handle(curlm, dlData->easy_handle);
			}
		}
	}

	bool aborted=false;
	int running=1, last=-1;
	while((running>0)&&(!aborted)) {
		CURLMcode ret = CURLM_CALL_MULTI_PERFORM;
		while(ret == CURLM_CALL_MULTI_PERFORM) {
			ret=curl_multi_perform(curlm, &running);
		}
		if ( ret == CURLM_OK ) {
//			showProcess(download, file);
			if (last!=running) { //count of running downloads changed
				aborted=processMessages(curlm, downloads);
				last=running++;
			}
		} else {
			LOG_ERROR("curl_multi_perform_error: %d", ret);
			aborted=true;
		}

		fd_set rSet;
		fd_set wSet;
		fd_set eSet;

		FD_ZERO(&rSet);
		FD_ZERO(&wSet);
		FD_ZERO(&eSet);
		int count=0;
		struct timeval t;
		t.tv_sec = 1;
		t.tv_usec = 0;
		curl_multi_fdset(curlm, &rSet, &wSet, &eSet, &count);
		//sleep for one sec / until something happened
		select(count+1, &rSet, &wSet, &eSet, &t);
	}
	if (!aborted) { // if download didn't fail, get file size reported in http-header
		double size=-1;
		for (unsigned i=0; i<downloads.size(); i++) {
			double tmp;
			curl_easy_getinfo(downloads[i]->easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &tmp);
			if (tmp>size) {
				size=tmp;
			}
		}
		//set download size if isn't set and we have a valid number
//		if ((size>0) && (download->size<0)) {
//  		download->size = size;
//		}

	}
//	showProcess(download, file);
	LOG("\n");

	if (!aborted) {
		LOG_INFO("download complete");
	}

	//close all open files
	for(it=download.begin(); it!=download.end(); ++it) {
		if ((*it)->file!=NULL)
			(*it)->file->Close();
	}
	for (unsigned i=0; i<downloads.size(); i++) {
		long timestamp;
		if (curl_easy_getinfo(downloads[i]->easy_handle, CURLINFO_FILETIME, &timestamp) == CURLE_OK) {
			if (downloads[i]->download->state != IDownload::STATE_FINISHED) //decrease local timestamp if download failed to force redownload next time
				timestamp--;
			downloads[i]->download->file->SetTimestamp(timestamp);
		}
		delete downloads[i];
	}

	downloads.clear();
	curl_multi_cleanup(curlm);
	return !aborted;
}


