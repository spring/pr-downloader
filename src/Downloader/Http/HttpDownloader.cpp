#include "HttpDownloader.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <zlib.h>
#include <time.h>
#include <iostream>
#include <sstream>


#include "lib/xmlrpc++/src/XmlRpc.h"
#include "FileSystem/File.h"
#include "FileSystem/IHash.h"
#include "FileSystem/HashMD5.h"
#include "FileSystem/HashCRC32.h"
#include "FileSystem/HashSHA1.h"

#include "Util.h"

time_t last_print;
time_t start_time;

/** *
	draw a nice download status-bar
*/
int progress_func(CHttpDownloader* ptr, double TotalToDownload, double NowDownloaded,
		  double TotalToUpload, double NowUploaded)
{
	time_t now=time(NULL);
	if (start_time==0)
		start_time=now;
	if (now!=last_print) { //check if 1 second is gone afters last update
		last_print=now;
	} else {
		if(TotalToDownload!=NowDownloaded) //print 100%
			return 0;
	}

	LOG_PROGRESS(NowDownloaded,TotalToDownload);

	return 0;
}


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	int written = fwrite(ptr, size, nmemb, stream);
	return written;
}

CHttpDownloader::CHttpDownloader()
{
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, PR_DOWNLOADER_AGENT);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	stats_filepos=1;
	stats_count=1;
}

CHttpDownloader::~CHttpDownloader()
{
	curl_easy_cleanup(curl);
	curl = NULL;
	curl_global_cleanup();
}

void CHttpDownloader::setCount(unsigned int count)
{
	this->stats_count=count;
}

unsigned int CHttpDownloader::getCount()
{
	return this->stats_count;
}

unsigned int CHttpDownloader::getStatsPos()
{
	return this->stats_filepos;
}

void CHttpDownloader::setStatsPos(unsigned int pos)
{
	this->stats_filepos=pos;
}

bool CHttpDownloader::search(std::list<IDownload*>& res, const std::string& name, IDownload::category cat)
{
	LOG_DEBUG("%s", name.c_str()  );

	const std::string method(XMLRPC_METHOD);
	std::string category;
	XmlRpc::XmlRpcClient client(XMLRPC_HOST,XMLRPC_PORT, XMLRPC_URI);
	XmlRpc::XmlRpcValue arg;
	arg["springname"]=name;
	arg["torrent"]=true;
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
			LOG_ERROR("No category in result\n");
			return false;
		}
		std::string filename=fileSystem->getSpringDir();
		std::string category=resfile["category"];
		filename+=PATH_DELIMITER;
		if (category=="map")
			filename+="maps";
		else if (category=="game")
			filename+="games";
		else
			LOG_DEBUG("Unknown Category %s", category.c_str());
		filename+=PATH_DELIMITER;
		if ((resfile["mirrors"].getType()!=XmlRpc::XmlRpcValue::TypeArray) ||
		    (resfile["filename"].getType()!=XmlRpc::XmlRpcValue::TypeString)) {
			LOG_ERROR("Invalid type in result\n");
			return false;
		}
		filename.append(resfile["filename"]);
		IDownload* dl=new IDownload(filename);
		XmlRpc::XmlRpcValue mirrors = resfile["mirrors"];
		for(int j=0; j<mirrors.size(); j++) {
			if (mirrors[j].getType()!=XmlRpc::XmlRpcValue::TypeString) {
				LOG_ERROR("Invalid type in result\n");
				return false;
			}

			dl->addMirror(mirrors[j]);
		}
		//torrent data avaiable
		if (resfile["torrent"].getType()==XmlRpc::XmlRpcValue::TypeString) { //FIXME: this is a bug in the xml-rpc interface, it should return <base64> but returns <string>
			std::string base64=resfile["torrent"];
			std::string binary;
			base64_decode(base64, binary);
			fileSystem->parseTorrent(binary.c_str(),binary.size(), dl);
		} else if(resfile["torrent"].getType()==XmlRpc::XmlRpcValue::TypeBase64) {
			XmlRpc::XmlRpcValue::BinaryData tmp= resfile["torrent"];
			std::string str;
			for(unsigned i=0; i<tmp.size(); i++)
				str.append(&tmp[i]);
			std::string binary;
			base64_decode(str, binary);
			fileSystem->parseTorrent(binary.c_str(), binary.size(), dl);
		}
		if (resfile["md5"].getType()==XmlRpc::XmlRpcValue::TypeString) {
			dl->hash=new HashMD5();
			dl->hash->Set(resfile["md5"]);
		}
		res.push_back(dl);
	}
	return true;
}

std::string CHttpDownloader::escapeUrl(const std::string& url)
{
	std::string res;

	for(unsigned int i=0; i<url.size(); i++) {
		if (url[i]==' ')
			res.append("%20");
		else
			res.append(1,url[i]);
	}
	return res;
}

size_t multi_write_data(void *ptr, size_t size, size_t nmemb, CHttpDownloader::download_data* data)
{
	return data->file->Write((const char*)ptr, size*nmemb, data->piece);
}

bool CHttpDownloader::getRange(std::string& range, int piece, int piecesize, int filesize)
{
	std::ostringstream s;
	s << (int)(piecesize*piece) <<"-"<< (piecesize*piece) + piecesize-1;
	range=s.str();
//	LOG("getRange: %s\n", range.c_str());
	return true;
}

void CHttpDownloader::showProcess(IDownload* download, CFile& file)
{
	unsigned long now=getTime();
	if(now>lastprogress) {
		lastprogress=now;
	} else
		return;
	int done=0;
	if(download->pieces.size()<=0) {
		done=file.GetPiecePos();
	} else {
		for(unsigned i=0; i<download->pieces.size(); i++) {
			switch(download->pieces[i].state) {
			case IDownload::STATE_FINISHED:
				done+=file.GetPieceSize(i);
				break;
			case IDownload::STATE_DOWNLOADING:
				done+=file.GetPiecePos(i);
				break;
			default:
				break;
			}
		}
	}
	LOG_PROGRESS(done, download->size);
}

int CHttpDownloader::verifyAndGetNextPiece(CFile& file, IDownload* download)
{
	//verify file by md5 if pieces.size == 0
	if((download->hash!=NULL) && (download->hash->isSet()) && (download->pieces.size()<=0)) {
		HashMD5 md5=HashMD5();
		file.Hash(md5);
		if (md5.compare(download->hash)) {
			LOG_INFO("md5 correct: %s\n", md5.toString().c_str());
			download->state=IDownload::STATE_FINISHED;
			return -1;
		} else {
			LOG_ERROR("md5 sum missmatch %s %s\n", download->hash->toString().c_str(), md5.toString().c_str());
		}
	}

	HashSHA1 sha1=HashSHA1();
	int pieceNum=-1;
	unsigned alreadyDl=0;
	for(unsigned i=0; i<download->pieces.size(); i++ ) { //find first not downloaded piece
		if (download->pieces[i].state==IDownload::STATE_NONE) {
			if (download->pieces[i].sha->isSet()) { //reuse piece, if checksum is fine
				file.Hash(sha1, i);
//	LOG("bla %s %s\n", sha1.toString().c_str(), download.pieces[i].sha->toString().c_str());
				if (sha1.compare(download->pieces[i].sha)) {
//					LOG_DEBUG("piece %d has already correct checksum, reusing", i);
					download->pieces[i].state=IDownload::STATE_FINISHED;
					showProcess(download, file);
					alreadyDl++;
					continue;
				}
			}
			pieceNum=i;
			break;
		}
	}

	//all pieces verified, mark whole file so, too
	if ((download->pieces.size()>0) && (alreadyDl==download->pieces.size()))
		download->state=IDownload::STATE_FINISHED;
	return pieceNum;
}

bool CHttpDownloader::setupDownload(CFile& file, download_data* piece, IDownload* download, int mirror)
{
	int pieceNum=verifyAndGetNextPiece(file, download);
	if (download->state==IDownload::STATE_FINISHED)
		return false;

	piece->file=&file;
	piece->piece=pieceNum;
	if (piece->easy_handle==NULL) {
		piece->easy_handle=curl_easy_init();
	} else {
		curl_easy_reset(piece->easy_handle);
	}
	CURL* curle= piece->easy_handle;
	piece->url=download->getMirror(mirror);
	curl_easy_setopt(curle, CURLOPT_WRITEFUNCTION, multi_write_data);
	curl_easy_setopt(curle, CURLOPT_WRITEDATA, piece);
	curl_easy_setopt(curle, CURLOPT_USERAGENT, PR_DOWNLOADER_AGENT);
	curl_easy_setopt(curle, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curle, CURLOPT_NOPROGRESS, 1L);
//	curl_easy_setopt(curle, CURLOPT_PROGRESSFUNCTION, progress_func);
//	curl_easy_setopt(curle, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curle, CURLOPT_URL, escapeUrl(piece->url).c_str());

	if ((download->size>0) && (pieceNum>=0)) { //don't set range, if size unknown
		std::string range;
		if (!getRange(range, pieceNum, download->piecesize, download->size )) {
			LOG_ERROR("Error getting range for download");
			return false;
		}
		//set range for request, format is <start>-<end>
		curl_easy_setopt(curle, CURLOPT_RANGE, range.c_str());
		download->pieces[pieceNum].state=IDownload::STATE_DOWNLOADING;
	}
	return true;
}

CHttpDownloader::download_data* CHttpDownloader::getDataByHandle(const std::vector <download_data*>& downloads, const CURL* easy_handle) const
{
	for(int i=0; i<(int)downloads.size(); i++) { //search corresponding data structure
		if (downloads[i]->easy_handle == easy_handle) {
			return downloads[i];
		}
	}
	return NULL;
}

bool CHttpDownloader::processMessages(CURLM* curlm, std::vector <download_data*>& downloads, IDownload* download, CFile& file)
{
	int msgs_left;
	HashSHA1 sha1;
	bool aborted=false;
	while(struct CURLMsg* msg=curl_multi_info_read(curlm, &msgs_left)) {
		switch(msg->msg) {
		case CURLMSG_DONE: { //a piece has been downloaded, verify it
			CHttpDownloader::download_data* data=getDataByHandle(downloads, msg->easy_handle);
			switch(msg->data.result) {
			case CURLE_OK:
				break;
			case CURLE_HTTP_RETURNED_ERROR: //some 4* HTTP-Error (file not found, access denied,...)
			default:
				LOG_ERROR("CURL error(%d): %s (%s)\n",msg->msg, curl_easy_strerror(msg->data.result), data->url.c_str());
				//FIXME: memleaks...
				return false;
			}

			if (data==NULL) {
				LOG_ERROR("Couldn't find download in download list\n");
				return false;
			}
			if (data->piece<0) { //download without pieces
				LOG("download finished\n");
				return false;
			}
			assert(data->file!=NULL);
			assert(data->piece<(int)download->pieces.size());
			if (download->pieces[data->piece].sha->isSet()) {
				data->file->Hash(sha1, data->piece);
				if (sha1.compare(download->pieces[data->piece].sha)) { //piece valid
					download->pieces[data->piece].state=IDownload::STATE_FINISHED;
				} else {
					download->pieces[data->piece].state=IDownload::STATE_FINISHED;
					LOG_ERROR("Invalid piece %d retrieved %s\n",data->piece,  sha1.toString().c_str());
					LOG_ERROR("%s\n", download->pieces[data->piece].sha->toString().c_str());
					LOG_ERROR("implement me! (redownload from different mirror)\n");
					aborted=true;
					break; //FIXME: implement
				}
			} else {
				LOG_INFO("sha1 checksum seems to be not set, can't check received piece %d\n", data->piece);
			}
			showProcess(download, file);
			//remove easy handle, as its finished
			curl_easy_cleanup(data->easy_handle);
			curl_multi_remove_handle(curlm, data->easy_handle);
			data->easy_handle=NULL;
			//piece finished / failed, try a new one
			//TODO: dynamic use mirrors
			//TODO: remove mirror when a mirror is slow/sent broken data and other mirrors are faster
			/*
								double dlSpeed;
								curl_easy_getinfo(data->easy_handle, CURLINFO_SPEED_DOWNLOAD, &dlSpeed);
								LOG("speed %.0f KB/s\n", dlSpeed/1024);
			*/
			if (!setupDownload(file, data, download, 0)) {
				LOG_INFO("No piece found, all pieces finished / currently downloading\n");
				break;
			}
			int ret=curl_multi_add_handle(curlm, data->easy_handle);
			if (ret!=CURLM_OK) {
				LOG_ERROR("curl_multi_perform_error: %d %d\n", ret, CURLM_BAD_EASY_HANDLE);
			}
			break;
		}
		default:
			LOG_ERROR("Unhandled message %d\n", msg->msg);
		}
	}
	return aborted;
}

bool CHttpDownloader::download(IDownload* download)
{

	const int count=std::min(MAX_PARALLEL_DOWNLOADS, std::max(1, std::min((int)download->pieces.size(), download->getMirrorCount()))); //count of parallel downloads
	if(download->getMirrorCount()<=0) {
		LOG_ERROR("No mirrors found\n");
		return false;
	}
	LOG_INFO("Using %d parallel downloads\n", count);

	CURLM* curlm=curl_multi_init();
	std::vector <download_data*> downloads;
	CFile file=CFile(download->name, download->size, download->piecesize);
	for(int i=0; i<count; i++) {
		download_data* dlData=new download_data();
		if (!setupDownload(file, dlData, download, i)) { //no piece found (all pieces already downloaded), skip
			delete dlData;
			if (download->state==IDownload::STATE_FINISHED) {
				LOG_INFO("finished\n");
				return true;
			} else {
				LOG_ERROR("no piece found\n");
				return false;
			}
		} else {
			downloads.push_back(dlData);
			curl_multi_add_handle(curlm, downloads[i]->easy_handle);
		}
	}

	bool aborted=false;
	int running=1, last=-1;
	while((running>0)&&(!aborted)) {
		CURLMcode ret=curl_multi_perform(curlm, &running);
		if (ret!=CURLM_OK) {
			LOG_ERROR("curl_multi_perform_error: %d\n", ret);
			aborted=true;
		}
		if (last!=running) { //count of running downloads changed
			aborted=processMessages(curlm, downloads, download, file);
		}
	}
	if (download->state==IDownload::STATE_FINISHED) {
		LOG_INFO("download complete\n");
	}

	lastprogress=0; //force progressbar to show 100%
	showProcess(download, file);
	LOG("\n");
	for (unsigned i=0; i<downloads.size(); i++) {
		delete downloads[i];
	}
	downloads.clear();
	curl_multi_cleanup(curlm);
	file.Close();
	return true;
}
