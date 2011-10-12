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


#include "xmlrpc++/src/XmlRpc.h"
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

bool CHttpDownloader::search(std::list<IDownload>& res, const std::string& name, IDownload::category cat)
{
	LOG_DEBUG("%s", name.c_str()  );

	const std::string method("springfiles.search");
	std::string category;

	XmlRpc::XmlRpcClient client("springfiles.com", 80, "http://springfiles.com/xmlrpc.php");
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
		IDownload dl=IDownload(filename);
		XmlRpc::XmlRpcValue mirrors = resfile["mirrors"];
		for(int j=0; j<mirrors.size(); j++) {
			if (mirrors[j].getType()!=XmlRpc::XmlRpcValue::TypeString) {
				LOG_ERROR("Invalid type in result\n");
				return false;
			}

			dl.addMirror(mirrors[j]);
		}
		//torrent data avaiable
		if (resfile["torrent"].getType()==XmlRpc::XmlRpcValue::TypeString) {
			std::string base64=resfile["torrent"];
			std::string binary;
			base64_decode(base64, binary); //FIXME: this is a bug int the xml-rpc interface, it should return <base64> but returns <string>
			fileSystem->parseTorrent(binary.c_str(),binary.size(),  dl);
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


bool CHttpDownloader::download(IDownload& download)
{
//FIXME: enable that
	if (download.getMirrorCount()>1)
		return parallelDownload(download);

	LOG_DEBUG("%s",download.name.c_str());
	last_print = 0;
	start_time = 0;
	CURLcode res=CURLE_OK;

	LOG_INFO("Using http\n");
	LOG_DOWNLOAD(download.getUrl().c_str());  //destination is download.name.c_str()

	//FIXME: use etag/timestamp as remote file could be modified
	/*
		if (fileSystem->fileExists(download.name)){
			//TODO: validate file
			INFO("file already downloaded: \"%s\"\n",download.name.c_str());
			return true;
		}
	*/

	if (!fileSystem->directoryExists(download.name)) {
		fileSystem->createSubdirs(download.name);
	}

	if (!curl) {
		LOG_ERROR("Error initializing curl");
		return false;
	}

	std::string temp = download.name + ".download";

	FILE* fp = fopen(temp.c_str() ,"wb+");
	if (fp==NULL) {
		LOG_ERROR("Could not open %s\n",temp.c_str());
		return false;
	}
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA ,this);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_URL, escapeUrl(download.getUrl()).c_str());
	res = curl_easy_perform(curl);
	fclose(fp);
	LOG("\n"); //new line because of downloadbar
	if (res!=0) {
		LOG_ERROR("Failed to download %s\n",download.getUrl().c_str());
		unlink(temp.c_str());
		return false;
	}

	if(rename(temp.c_str(),download.name.c_str())) {
		LOG_ERROR("Could not write to %s\n",download.name.c_str());
		return false;
	}

	return true;
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


bool CHttpDownloader::getPiece(CFile& file, download_data* piece, IDownload& download, int mirror)
{
	int pieceNum=-1;
	for(int i=0; i<(int)download.pieces.size(); i++ ) { //find first not downloaded piece
		assert(i<download.pieces.size());
		if ( (download.pieces[i].state==IDownload::STATE_NONE) ) {
			pieceNum=i;
			break;
		}
	}
	if (pieceNum<0) //all pieces downloaded or in state of downloading
		return false;
	piece->file=&file;
	piece->piece=pieceNum;
	CURL* curle= piece->easy_handle;
	curl_easy_reset(curle);
	curl_easy_setopt(curle, CURLOPT_WRITEFUNCTION, multi_write_data);
	curl_easy_setopt(curle, CURLOPT_WRITEDATA, piece);
	curl_easy_setopt(curle, CURLOPT_USERAGENT, PR_DOWNLOADER_AGENT);
	curl_easy_setopt(curle, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curle, CURLOPT_NOPROGRESS, 1L);
//	curl_easy_setopt(curle, CURLOPT_PROGRESSFUNCTION, progress_func);
//	curl_easy_setopt(curle, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curle, CURLOPT_URL, escapeUrl(download.getMirror(mirror)).c_str());
	std::string range;
	if (!getRange(range, pieceNum, download.piecesize, download.size )) {
		LOG_ERROR("Error getting range for download");
		return false;
	}
	//set range for request, format is <start>-<end>
	curl_easy_setopt(curle, CURLOPT_RANGE, range.c_str());
	download.pieces[pieceNum].state=IDownload::STATE_DOWNLOADING;
	return true;
}

bool CHttpDownloader::parallelDownload(IDownload& download)
{
	CFile file=CFile(download.name, download.size, download.piecesize);
	HashMD5 md5=HashMD5();
	HashSHA1 sha1=HashSHA1();
	std::list<IHash*> hashes;
	hashes.clear();
	hashes.push_back(&md5);
	hashes.push_back(&sha1);
	std::vector <download_data*> downloads;
	CURLM* curlm=curl_multi_init();
	const int count=std::min((int)download.pieces.size(), download.getMirrorCount()); //count of parallel downloads
	if(count<=0) {
		LOG_ERROR("No mirrors found or counts of pieces==0 (count=%d)\n", download.getMirrorCount());
		return false;
	}
	for(int i=0; i<count; i++) {
		download_data* dlData=new download_data();
		if (!getPiece(file, dlData, download, i)) {
			LOG_ERROR("couldn't get piece\n");
			return false;
		}
		downloads.push_back(dlData);
		curl_multi_add_handle(curlm, downloads[i]->easy_handle);
	}
	int running, last=1;
	do {
		/*TODO:
			add new download when piece is finished
			(remove mirror when a mirror is slow and other mirrors are faster)
		*/
		CURLMcode ret=curl_multi_perform(curlm, &running);
		if (ret!=CURLM_OK) {
			LOG_ERROR("curl_multi_perform_error: %d\n", ret);
		}
		if ((last!=0) && (last!=running)) { //count of running downloads changed
			int msgs_left;
			while(struct CURLMsg* msg=curl_multi_info_read(curlm, &msgs_left)) {
				switch(msg->msg) {
				case CURLMSG_DONE: { //a piece has been downloaded, verify it
					if ( msg->data.result!=CURLE_OK) {
						LOG_ERROR("CURLcode: %d\n", msg->data.result);

					}
					download_data* data=NULL;
					for(int i=0; i<downloads.size(); i++) { //search corresponding data structure
						if (downloads[i]->easy_handle == msg->easy_handle) {
							data=downloads[i];
							break;
						}
					}

					if (data==NULL) {
						LOG_ERROR("Couldn't find download in download list\n");
						return false;
					}
					assert(data->file!=NULL);
					assert(data->piece<download.pieces.size());
					data->file->Hash(hashes, data->piece); //TODO: create hash + compare with download.piece[i].hashes
					if (download.pieces[data->piece].sha[0]==0) {
						LOG_INFO("sha1 checksum seems to be not set, can't check received piece %d\n", data->piece);
					}
					if ( (download.pieces[data->piece].sha[0]==0)
					     || (sha1.compare((unsigned char*)download.pieces[data->piece].sha, 5))) { //piece valid
						download.pieces[data->piece].state=IDownload::STATE_FINISHED;
					} else {
						download.pieces[data->piece].state=IDownload::STATE_NONE;
						LOG_ERROR("Invalid piece retrieved\n %s", sha1.toString().c_str());
						//FIXME: mark mirror as broken (to avoid endless loops!)
					}
					//remove easy handle, as its finished
					curl_multi_remove_handle(curlm, data->easy_handle);
					//piece finished / failed, try a new one
					//TODO: dynamic use mirrors
					/*
										double dlSpeed;
										curl_easy_getinfo(data->easy_handle, CURLINFO_SPEED_DOWNLOAD, &dlSpeed);
										LOG("speed %.0f KB/s\n", dlSpeed/1024);
					*/
					if (!getPiece(file, data, download, 0)) {
						LOG_INFO("No piece found, all pieces finished / currently downloading\n");
						break;
					}
					ret=curl_multi_add_handle(curlm, data->easy_handle);
					if (ret!=CURLM_OK) {
						LOG_ERROR("curl_multi_perform_error: %d %d\n", ret, CURLM_BAD_EASY_HANDLE);
					}
					running++;
					break;
				}
				default:
					LOG_ERROR("Unhandled message %d\n", msg->msg);
				}
			}
		}
	} while(running>0);
	curl_multi_cleanup(curlm);
	file.Close();
	return true;
}
