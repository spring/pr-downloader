#include "HttpDownloader.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <zlib.h>
#include <time.h>
#include <iostream>

#include "xmlrpc++/src/XmlRpc.h"


#include "../../Util.h"

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
	curl=NULL;
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
	DEBUG_LINE("%s", name.c_str()  );

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
			DEBUG_LINE("Unknown Category %s", category.c_str());
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
		if (resfile["torrent"].getType()==XmlRpc::XmlRpcValue::TypeString){
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
/*
	if (download.getMirrorCount()>1)
		parallelDownload(download);
*/
	DEBUG_LINE("%s",download.name.c_str());
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

  if (!fileSystem->directoryExists(download.name)){
  	fileSystem->createSubdirs(download.name);
  }

	if (!curl) {
		LOG_ERROR("Error initializing curl");
		return false;
	}
	
  std::string temp = fileSystem->createTempFile();
	
	FILE* fp = fopen(temp.c_str() ,"wb+");
	if (fp==NULL) {
		LOG_ERROR("Could not open %s\n",download.name.c_str());
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
		unlink(download.name.c_str());
		return false;
	}
	
	if(rename(temp.c_str(),download.name.c_str())) {
    ERROR("Could not write to %s\n",download.name.c_str());
    return false;
	}
    
	return true;
}

bool CHttpDownloader::parallelDownload(IDownload& download){
	CURLM* curlm=curl_multi_init();
	const int count=download.mirror.size();
	for(int i=0; i<count; i++){
		CURL* curle = curl_easy_init();
		curl_easy_setopt(curle, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curle, CURLOPT_USERAGENT, PR_DOWNLOADER_AGENT);
		curl_easy_setopt(curle, CURLOPT_FAILONERROR, true);
		curl_easy_setopt(curle, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curle, CURLOPT_PROGRESSFUNCTION, progress_func);
		curl_easy_setopt(curle, CURLOPT_PROGRESSDATA, this);
		curl_easy_setopt(curle, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curle, CURLOPT_URL, escapeUrl(download.getMirror(i)).c_str());
		std::string range;
		if (!download.getRange(range)){
			LOG_ERROR("Error getting range for download");
			return false;
		}
		curl_easy_setopt(curle, CURLOPT_RANGE, range.c_str());
//		curl_easy_setopt(curle, CURLOPT_RETURNTRANSFER, true);
		curl_multi_add_handle(curlm, curle);
	}
	int running;
	do{
		/*TODO:
			add new download when piece is finished
			verify piece, when broken remove mirror from list
			(remove mirror when it is slow and count of mirros > 1)
		*/
		curl_multi_perform(curlm, &running);
	}while(running>0);
	return true;
}
