#include "HttpDownloader.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <zlib.h>


#include "xmlrpc++/src/XmlRpc.h"


#include "../../Util.h"


/** *
	draw a nice download status-bar
*/
int progress_func(CHttpDownloader* ptr, double TotalToDownload, double NowDownloaded,
				  double TotalToUpload, double NowUploaded) {
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

	const std::string serverUrl("http://new.springfiles.com/xmlrpc.php");
	const std::string method("springfiles.search");
	std::string category;
	std::string filename=fileSystem->getSpringDir();
	filename+=PATH_DELIMITER;
	switch(cat){
		case IDownload::CAT_MAPS:
			category="map";
			filename+="maps";
			break;
		case IDownload::CAT_MODS:
			category="game";
			filename+="games";
			break;
		default:{
			category="map";
		}
	}
	filename+=PATH_DELIMITER;

 	XmlRpc::XmlRpcClient client("new.springfiles.com", 80, "http://new.springfiles.com/xmlrpc.php");
	XmlRpc::XmlRpcValue arg;
	arg["filename"]=name;
	arg["category"]=category;
	XmlRpc::XmlRpcValue result;
	client.execute(method.c_str(),arg, result);

	if (result.getType()!=XmlRpc::XmlRpcValue::TypeStruct){
		return res;
	}

	if ((result["mirrors"].getType()!=XmlRpc::XmlRpcValue::TypeArray) ||
		(result["filename"].getType()!=XmlRpc::XmlRpcValue::TypeString)){
		printf("Invalid type in result\n");
		return res;
	}
	IDownload* dl=NULL;
	XmlRpc::XmlRpcValue mirrors = result["mirrors"];
	for(int j=0; j<mirrors.size(); j++){
		if (mirrors[j].getType()!=XmlRpc::XmlRpcValue::TypeString){
			printf("Invalid type in result\n");
			return res;
		}
		if (dl==NULL){
			dl=new IDownload(result["filename"],mirrors[j]);
		}
		dl->addMirror(mirrors[j]);
	}

	res->push_back(*dl);
	return res;
}

bool CHttpDownloader::download(IDownload& download) {
	DEBUG_LINE("%s",download.name.c_str());

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
	curl_easy_setopt(curl, CURLOPT_URL, download.url.c_str());
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
