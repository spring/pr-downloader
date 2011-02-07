#include "HttpDownloader.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <zlib.h>
#include <time.h>

#include "xmlrpc++/src/XmlRpc.h"


#include "../../Util.h"

time_t last_print;

/** *
	draw a nice download status-bar
*/
int progress_func(CHttpDownloader* ptr, double TotalToDownload, double NowDownloaded,
				  double TotalToUpload, double NowUploaded) {
	time_t now=time(NULL);
	fflush(stdout);
	if ((now-last_print)<=0)
		return 0;
	last_print=now;
	// how wide you want the progress meter to be
	int totaldotz=40;
	double fractiondownloaded;
	if (TotalToDownload>0)
		fractiondownloaded = NowDownloaded / TotalToDownload;
	else
		fractiondownloaded=0;
	// part of the progressmeter that's already "full"
	int dotz = fractiondownloaded * totaldotz;

	// create the "meter"
	printf("%5d/%5d ", ptr->getStatsPos(),ptr->getCount());
	printf("%3.0f%% [",fractiondownloaded*100);
	int ii=0;
	// part  that's full already
	for ( ; ii < dotz;ii++) {
		printf("=");
	}
	// remaining part (spaces)
	for ( ; ii < totaldotz;ii++) {
		printf(" ");
	}
	// and back to line begin - do not forget the fflush to avoid output buffering problems!
	printf("] %d/%d\r",(int)NowDownloaded,(int)TotalToDownload );
	fflush(stdout);
	return 0;
}


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	int written = fwrite(ptr, size, nmemb, stream);
	return written;
}

CHttpDownloader::CHttpDownloader() {
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	stats_filepos=1;
	stats_count=1;
}

CHttpDownloader::~CHttpDownloader() {
	curl_easy_cleanup(curl);
	curl=NULL;
}

void CHttpDownloader::setCount(unsigned int count) {
	this->stats_count=count;
}

unsigned int CHttpDownloader::getCount() {
	return this->stats_count;
}

unsigned int CHttpDownloader::getStatsPos() {
	return this->stats_filepos;
}

void CHttpDownloader::setStatsPos(unsigned int pos) {
	this->stats_filepos=pos;
}

std::list<IDownload>* CHttpDownloader::search(const std::string& name, IDownload::category cat) {
	DEBUG_LINE("%s", name.c_str()  );
	std::list<IDownload>* res;
	res=new std::list<IDownload>();

	const std::string method("springfiles.search");
	std::string category;

 	XmlRpc::XmlRpcClient client("springfiles.com", 80, "http://springfiles.com/xmlrpc.php");
	XmlRpc::XmlRpcValue arg;
	arg["springname"]=name;
	XmlRpc::XmlRpcValue result;
	client.execute(method.c_str(),arg, result);


	if (result.getType()!=XmlRpc::XmlRpcValue::TypeArray){
		return res;
	}

	for(int i=0; i<result.size(); i++){
		XmlRpc::XmlRpcValue resfile = result[i];

		if (resfile.getType()!=XmlRpc::XmlRpcValue::TypeStruct){
			return res;
		}
		if (resfile["category"].getType()!=XmlRpc::XmlRpcValue::TypeString){
			printf("No category in result\n");
			return res;
		}
		std::string filename=fileSystem->getSpringDir();
		filename+=PATH_DELIMITER;
		if (resfile["category"]=="Spring Maps")
			filename+="maps";
		else if (resfile["category"]=="Spring Game")
			filename+="games";
		filename+=PATH_DELIMITER;
		if ((resfile["mirrors"].getType()!=XmlRpc::XmlRpcValue::TypeArray) ||
			(resfile["filename"].getType()!=XmlRpc::XmlRpcValue::TypeString)){
			printf("Invalid type in result\n");
			return res;
		}
		filename.append(resfile["filename"]);
		IDownload* dl=NULL;
		XmlRpc::XmlRpcValue mirrors = resfile["mirrors"];
		for(int j=0; j<mirrors.size(); j++){
			if (mirrors[j].getType()!=XmlRpc::XmlRpcValue::TypeString){
				printf("Invalid type in result\n");
				return res;
			}
			if (dl==NULL){
				dl=new IDownload(mirrors[j],filename);
			}
			dl->addMirror(mirrors[j]);
		}

		res->push_back(*dl);
	}
	return res;
}

std::string CHttpDownloader::escapeUrl(const std::string& url){
	std::string res;

	for(unsigned int i=0; i<url.size(); i++){
		if (url[i]==' ')
			res.append("%20");
		else
			res.append(1,url[i]);
	}
	return res;
}


bool CHttpDownloader::download(IDownload& download) {
	DEBUG_LINE("%s",download.name.c_str());
	last_print = 0;
	CURLcode res=CURLE_OK;
	printf("Downloading %s to %s\n",download.url.c_str(), download.name.c_str());
	//FIXME: use etag/timestamp as remote file could be modified
	/*
		if (fileSystem->fileExists(download.name)){
			//TODO: validate file
			printf("file already downloaded: \"%s\"\n",download.name.c_str());
			return true;
		}
	*/

	if (!curl) {
		printf("Error initializing curl");
		return false;
	}
	FILE* fp = fopen(download.name.c_str() ,"wb+");
	if (fp==NULL) {
		printf("CHttpDownloader:: Could not open %s\n",download.name.c_str());
		return false;
	}
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA ,this);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_URL, escapeUrl(download.url).c_str());
	res = curl_easy_perform(curl);
	fclose(fp);
	printf("\n"); //new line because of downloadbar
	if (res!=0) {
		printf("error downloading %s\n",download.url.c_str());
		unlink(download.name.c_str());
		return false;
	}
	return true;
}
