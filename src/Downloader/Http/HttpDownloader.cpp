/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "HttpDownloader.h"
#include <json/reader.h>

#include "DownloadData.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/HashMD5.h"
#include "FileSystem/HashSHA1.h"
#include "Util.h"
#include "Logger.h"
#include "Downloader/Mirror.h"
#include "Downloader/CurlWrapper.h"
#include "lib/base64/base64.h"


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
#include <algorithm> //std::min, std::max

CHttpDownloader::CHttpDownloader()
{
}

CHttpDownloader::~CHttpDownloader()
{
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size*nmemb;
	std::string* res = static_cast<std::string*>(userp);
	res->append((char*)contents, realsize);
	return realsize;
}

//downloads url into res
bool CHttpDownloader::DownloadUrl(const std::string& url, std::string& res)
{
	CurlWrapper* curlw = new CurlWrapper();
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_URL, CurlWrapper::escapeUrl(url).c_str());
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curlw->GetHandle(), CURLOPT_WRITEDATA, (void *)&res);
	CURLcode curlres = curl_easy_perform(curlw->GetHandle());
	if (curlres != CURLE_OK) {
	        LOG_ERROR("Error in curl %s", curl_easy_strerror(curlres));
	}
	delete curlw;
	return curlres == CURLE_OK;
}

static std::string getRequestUrl(const std::string& name, DownloadEnum::Category cat)
{

	std::string url = HTTP_SEARCH_URL + std::string("?");
	if (cat != DownloadEnum::CAT_NONE) {
		url += "category=" + DownloadEnum::getCat(cat) + std::string("&");
	}
	return url + std::string("torrent=true&springname=") + name;
}

bool CHttpDownloader::ParseResult(const std::string& name, const std::string& json, std::list<IDownload*>& res)
{
	Json::Value result;   // will contains the root value after parsing.
	Json::Reader reader;
	const bool parsingSuccessful = reader.parse( json, result );
	if ( !parsingSuccessful ) {
		LOG_ERROR("Couldn't parse result: %s %s", reader.getFormattedErrorMessages().c_str(), json.c_str());
		return false;
	}

	if (!result.isArray()) {
		LOG_ERROR("Returned json isn't an array!");
		return false;
	}

	for(Json::Value::ArrayIndex i=0; i<result.size(); i++) {
		Json::Value resfile = result[i];

		if (!resfile.isObject()) {
			LOG_ERROR("Entry isn't object!");
			return false;
		}
		if (!resfile["category"].isString()) {
			LOG_ERROR("No category in result");
			return false;
		}
		if (!resfile["springname"].isString()) {
			LOG_ERROR("No springname in result");
			return false;
		}
		std::string filename=fileSystem->getSpringDir();
		std::string category=resfile["category"].asString();
		const std::string springname = resfile["springname"].asString();
		filename+=PATH_DELIMITER;

		if (category=="map") {
			filename+="maps";
		} else if (category=="game") {
			filename+="games";
		} else if (category.find("engine")==0) { // engine_windows, engine_linux, engine_macosx
			filename+="engine";
		} else
			LOG_ERROR("Unknown Category %s", category.c_str());
		filename+=PATH_DELIMITER;

		if ((!resfile["mirrors"].isArray()) ||
		    (!resfile["filename"].isString())) {
			LOG_ERROR("Invalid type in result");
			return false;
		}
		filename.append(resfile["filename"].asString());

		const DownloadEnum::Category cat = DownloadEnum::getCatFromStr(category);
		IDownload* dl=new IDownload(filename, springname, cat);
		Json::Value mirrors = resfile["mirrors"];
		for(Json::Value::ArrayIndex j=0; j<mirrors.size(); j++) {
			if (!mirrors[j].isString()) {
				LOG_ERROR("Invalid type in result");
			} else {
				dl->addMirror(mirrors[j].asString());
			}
		}

		if(resfile["torrent"].isString()) {
			const std::string torrent = base64_decode(resfile["torrent"].asString());
			fileSystem->parseTorrent(&torrent[0], torrent.size(), dl);
		}
		if (resfile["version"].isString()) {
			const std::string& version = resfile["version"].asString();
			dl->version = version;
		}
		if (resfile["md5"].isString()) {
			dl->hash=new HashMD5();
			dl->hash->Set(resfile["md5"].asString());
		}
		if (resfile["size"].isInt()) {
			dl->size=resfile["size"].asInt();
		}
		if (resfile["depends"].isArray()) {
			for(Json::Value::ArrayIndex i=0; i<resfile["depends"].size(); i++) {
				if (resfile["depends"][i].isString()) {
					const std::string &dep = resfile["depends"][i].asString();
					dl->addDepend(dep);
				}
			}
		}
		res.push_back(dl);
	}
	return true;
}

bool CHttpDownloader::search(std::list<IDownload*>& res, const std::string& name, DownloadEnum::Category cat)
{
	LOG_DEBUG("%s", name.c_str()  );
	std::string dlres;
	const std::string url = getRequestUrl(name, cat);
	if (!DownloadUrl(url, dlres)) {
		LOG_ERROR("Error downloading %s %s", url.c_str(), dlres.c_str());
	        return false;
	}
	return ParseResult(name, dlres, res);
}

size_t multi_write_data(void *ptr, size_t size, size_t nmemb, DownloadData* data)
{
	//LOG_DEBUG("%d %d",size,  nmemb);
	if (!data->got_ranges) {
		LOG_INFO("Server refused ranges"); // The server refused ranges , download only from this piece , overwrite from 0 , and drop everything else

		data->download->write_only_from = data;
		data->got_ranges = true; //Silence the error
	}
	if ( data->download->write_only_from != NULL && data->download->write_only_from != data )
		return size*nmemb;
	else if ( data->download->write_only_from != NULL ) {
		return data->download->file->Write((const char*)ptr, size*nmemb, 0);
	}
	return data->download->file->Write((const char*)ptr, size*nmemb, data->start_piece);
}

size_t multiHeader(void *ptr, size_t size, size_t nmemb, DownloadData *data)
{
	if(data->download->pieces.empty()) { //no chunked transfer, don't check headers
		LOG_DEBUG("Unchunked transfer!");
		data->got_ranges = true;
		return size*nmemb;
	}
	const std::string buf((char*)ptr, size*nmemb-1);
	int start, end, total;
	int count = sscanf(buf.c_str(), "Content-Range: bytes %d-%d/%d", &start, &end, &total);
	if(count == 3) {
		int piecesize = data->download->file->GetPiecesSize(data->pieces);
		if (end-start+1!=piecesize ) {
			LOG_DEBUG("piecesize %d doesn't match server size: %d", piecesize, end-start+1);
			return -1;
		}
		data->got_ranges = true;
	}
	LOG_DEBUG("%s", buf.c_str());
	return size*nmemb;
}

bool CHttpDownloader::getRange(std::string& range, int start_piece, int num_pieces, int piecesize)
{
	std::ostringstream s;
	s << (int)(piecesize*start_piece) <<"-"<< (piecesize*start_piece) + piecesize*num_pieces-1;
	range=s.str();
	LOG_DEBUG("%s", range.c_str());
	return true;
}

void CHttpDownloader::showProcess(IDownload* download, bool force)
{
	int done = download->getProgress();
	int size = download->size;
	if (listener != NULL) {
		listener(done, size);
	}
	LOG_PROGRESS(done, size, force);
}

std::vector< unsigned int > CHttpDownloader::verifyAndGetNextPieces(CFile& file, IDownload* download)
{
	std::vector<unsigned int> pieces;
	if (download->state == IDownload::STATE_FINISHED) {
		return pieces;
	}
	//verify file by md5 if pieces.size == 0
	if((download->pieces.empty()) && (download->hash!=NULL) && (download->hash->isSet())) {
		HashMD5 md5=HashMD5();
		file.Hash(md5);
		if (md5.compare(download->hash)) {
			LOG_INFO("md5 correct: %s", md5.toString().c_str());
			download->state=IDownload::STATE_FINISHED;
			showProcess(download, true);
			return pieces;
		} else {
			LOG_INFO("md5 sum missmatch %s %s", download->hash->toString().c_str(), md5.toString().c_str());
		}
	}

	HashSHA1 sha1=HashSHA1();
	unsigned alreadyDl=0;
	for(unsigned i=0; i<download->pieces.size(); i++ ) { //find first not downloaded piece
		showProcess(download, false);
		if (download->pieces[i].state==IDownload::STATE_FINISHED) {
			alreadyDl++;
			LOG_DEBUG("piece %d marked as downloaded", i);
			if ( pieces.size() > 0 )
				break;//Contiguos non-downloaded area finished
			continue;
		} else if (download->pieces[i].state==IDownload::STATE_NONE) {
			if ((download->pieces[i].sha->isSet()) && (!file.IsNewFile())) { //reuse piece, if checksum is fine
				file.Hash(sha1, i);
//	LOG("bla %s %s", sha1.toString().c_str(), download.pieces[i].sha->toString().c_str());
				if (sha1.compare(download->pieces[i].sha)) {
					LOG_DEBUG("piece %d has already correct checksum, reusing", i);
					download->pieces[i].state=IDownload::STATE_FINISHED;
					showProcess(download, true);
					alreadyDl++;
					if ( pieces.size() > 0 )
						break;//Contiguos non-downloaded area finished
					continue;
				}
			}
			pieces.push_back(i);
			if ( pieces.size() == download->pieces.size()/download->parallel_downloads )
				break;
		}
	}
	if (pieces.size() == 0 && download->pieces.size() != 0) {
		LOG_DEBUG("Finished\n");
		download->state=IDownload::STATE_FINISHED;
		showProcess(download, true);
	}
	LOG_DEBUG("Pieces to download: %d\n",pieces.size());
	return pieces;
}

static int progress_func(DownloadData* data, double total, double done, double, double)
{
	data->download->progress = done;
	if (IDownloader::listener != NULL) {
		IDownloader::listener(done, total);
	}
	if (data->got_ranges) {
		LOG_PROGRESS(done, total, done >= total);
	}
	return 0;
}

bool CHttpDownloader::setupDownload(DownloadData* piece)
{
	std::vector<unsigned int> pieces = verifyAndGetNextPieces(*(piece->download->file), piece->download);
	if (piece->download->state==IDownload::STATE_FINISHED)
		return false;
	if ( piece->download->file ) {
		piece->download->size = piece->download->file->GetPieceSize(-1);
		LOG_DEBUG("Size is %d",piece->download->size);
	}
	piece->start_piece = pieces.size() > 0 ? pieces[0] : -1;
	assert(piece->download->pieces.size()<=0 || piece->start_piece >=0);
	piece->pieces = pieces;
	if (piece->curlw==NULL) {
		piece->curlw = new CurlWrapper();
	} else {
		delete piece->curlw;
		piece->curlw = new CurlWrapper();
	}

	CURL* curle = piece->curlw->GetHandle();
	piece->mirror=piece->download->getFastestMirror();
	if (piece->mirror==NULL) {
		LOG_ERROR("No mirror found");
		return false;
	}

	curl_easy_setopt(curle, CURLOPT_WRITEFUNCTION, multi_write_data);
	curl_easy_setopt(curle, CURLOPT_WRITEDATA, piece);
	curl_easy_setopt(curle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curle, CURLOPT_PROGRESSDATA, piece);
	curl_easy_setopt(curle, CURLOPT_PROGRESSFUNCTION, progress_func);
	curl_easy_setopt(curle, CURLOPT_URL, CurlWrapper::escapeUrl(piece->mirror->url).c_str());

	if ((piece->download->size>0) && (piece->start_piece>=0) && piece->download->pieces.size() > 0) { //don't set range, if size unknown
		std::string range;
		if (!getRange(range, piece->start_piece, piece->pieces.size() , piece->download->piecesize)) {
			LOG_ERROR("Error getting range for download");
			return false;
		}
		//set range for request, format is <start>-<end>
		if ( !(piece->start_piece == 0 && piece->pieces.size() == piece->download->pieces.size()) )
			curl_easy_setopt(curle, CURLOPT_RANGE, range.c_str());
		//parse server response	header as well
		curl_easy_setopt(curle, CURLOPT_HEADERFUNCTION, multiHeader);
		curl_easy_setopt(curle, CURLOPT_WRITEHEADER, piece);
		for ( std::vector<unsigned int>::iterator it = piece->pieces.begin(); it != piece->pieces.end(); ++it )
			piece->download->pieces[*it].state=IDownload::STATE_DOWNLOADING;
	} else { //
		LOG_DEBUG("single piece transfer");
		piece->got_ranges = true;

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
	for(size_t i=0; i<downloads.size(); i++) { //search corresponding data structure
		if (downloads[i]->curlw == nullptr) { //inactive download
			continue;
		}
		if (downloads[i]->curlw->GetHandle() == easy_handle) {
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
				LOG_ERROR("CURL error(%d:%d): %s %d (%s)",msg->msg, msg->data.result, curl_easy_strerror(msg->data.result), http_code, data->mirror->url.c_str());
				if (data->start_piece>=0) {
					data->download->pieces[data->start_piece].state=IDownload::STATE_NONE;
				}
				data->mirror->status=Mirror::STATUS_BROKEN;
				//FIXME: cleanup curl handle here + process next dl
			}
			if (data==NULL) {
				LOG_ERROR("Couldn't find download in download list");
				return false;
			}
			if (data->start_piece<0) { //download without pieces
				return false;
			}
			assert(data->download->file!=NULL);
			assert(data->start_piece< (int)data->download->pieces.size());
			for ( std::vector<unsigned int>::iterator it = data->pieces.begin(); it != data->pieces.end(); ++it ) {
				if (data->download->pieces[*it].sha->isSet()) {
					data->download->file->Hash(sha1, *it);

					if (sha1.compare(data->download->pieces[*it].sha)) { //piece valid

						data->download->pieces[*it].state=IDownload::STATE_FINISHED;
						showProcess(data->download, true);
						// LOG("piece %d verified!", data->piece);
					} else { // piece download broken, mark mirror as broken (for this file)
						data->download->pieces[*it].state=IDownload::STATE_NONE;
						data->mirror->status=Mirror::STATUS_BROKEN;
						//FIXME: cleanup curl handle here + process next dl
						LOG_ERROR("Piece %d is invalid",*it);
					}
				} else {
					LOG_INFO("sha1 checksum seems to be not set, can't check received piece %d-%d", data->start_piece,data->pieces.size());
				}
			}
			//get speed at which this piece was downloaded + update mirror info
			double dlSpeed;
			curl_easy_getinfo(data->curlw->GetHandle(), CURLINFO_SPEED_DOWNLOAD, &dlSpeed);
			data->mirror->UpdateSpeed(dlSpeed);
			if (data->mirror->status == Mirror::STATUS_UNKNOWN) //set mirror status only when unset
				data->mirror->status=Mirror::STATUS_OK;

			//remove easy handle, as its finished
			curl_multi_remove_handle(curlm, data->curlw->GetHandle());
			delete data->curlw;
			data->curlw=NULL;
			LOG_INFO("piece finished");
			//piece finished / failed, try a new one
			if (!setupDownload(data)) {
				LOG_DEBUG("No piece found, all pieces finished / currently downloading");
				break;
			}
			int ret=curl_multi_add_handle(curlm, data->curlw->GetHandle());
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

bool CHttpDownloader::download(std::list<IDownload*>& download, int max_parallel)
{
	std::vector <DownloadData*> downloads;
	CURLM* curlm=curl_multi_init();
	for(IDownload* dl: download) {
		if (dl->state == IDownload::STATE_FINISHED) {
			continue;
		}
		if (dl->dltype != IDownload::TYP_HTTP) {
			LOG_DEBUG("skipping non http-dl")
			continue;
		}
		const int count=std::min(max_parallel, std::max(1, std::min((int)dl->pieces.size(), dl->getMirrorCount()))); //count of parallel downloads
		if(dl->getMirrorCount()<=0) {
			LOG_ERROR("No mirrors found");
			return false;
		}
		LOG_DEBUG("Using %d parallel downloads", count);
		dl->parallel_downloads = count;
		if (dl->file == nullptr) {
			dl->file = new CFile();
			if (!dl->file->Open(dl->name, dl->size, dl->piecesize)) {
				delete dl->file;
				dl->file = nullptr;
				return false;
			}
		}
		for(int i=0; i<count; i++) {
			DownloadData* dlData=new DownloadData();
			dlData->download=dl;
			if (!setupDownload(dlData)) { //no piece found (all pieces already downloaded), skip
				delete dlData;
				if (dl->state!=IDownload::STATE_FINISHED) {
					LOG_ERROR("no piece found");
					return false;
				}
			} else {
				downloads.push_back(dlData);
				curl_multi_add_handle(curlm, dlData->curlw->GetHandle());
			}
		}
	}
	if (downloads.empty()) {
		LOG_DEBUG("Nothing to download!");
		//close all open files
		for(IDownload* dl: download) {
			if (dl->file != nullptr) {
				dl->file->Close();
				delete dl->file;
				dl->file = nullptr;
			}
		}
		return true;
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
			if (downloads[i]->curlw == nullptr)
				continue;
			double tmp;
			curl_easy_getinfo(downloads[i]->curlw->GetHandle(), CURLINFO_CONTENT_LENGTH_DOWNLOAD, &tmp);
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
		LOG_DEBUG("download complete");
	}

	//close all open files
	for(IDownload* dl: download) {
		if (dl->file != nullptr) {
			dl->file->Close();
		}
	}
	for (size_t i=0; i<downloads.size(); i++) {
		long timestamp;
		if ((downloads[i]->curlw != nullptr) && curl_easy_getinfo(downloads[i]->curlw->GetHandle(), CURLINFO_FILETIME, &timestamp) == CURLE_OK) {
			if (downloads[i]->download->state != IDownload::STATE_FINISHED) //decrease local timestamp if download failed to force redownload next time
				timestamp--;
			downloads[i]->download->file->SetTimestamp(timestamp);
			delete downloads[i]->download->file;
			downloads[i]->download->file = nullptr;
		}
		delete downloads[i];
	}

	downloads.clear();
	curl_multi_cleanup(curlm);
	return !aborted;
}


