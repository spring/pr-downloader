
#include "HttpDownloader.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <zlib.h>
#include "../../Util.h"


/** *
	draw a nice download status-bar
*/
int progress_func(CHttpDownloader* ptr, double TotalToDownload, double NowDownloaded,
                    double TotalToUpload, double NowUploaded){
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

bool CHttpDownloader::download(const std::string& Url, const std::string& filename, int pos){
	CURLcode res=CURLE_OK;
    printf("CHttpDownloader::download %s to %s\n",Url.c_str(), filename.c_str());

	if(!curl) {
		printf("Error initializing curl");
		return false;
	}
	FILE* fp = fopen(filename.c_str() ,"wb+");
	if (fp<=NULL){
        printf("CHttpDownloader:: Could not open %s\n",filename.c_str());
		return false;
	}
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA ,this);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_URL, Url.c_str());
	res = curl_easy_perform(curl);
	fclose(fp);
	printf("\n"); //new line because of downloadbar
	if (res!=0){
		printf("error downloading %s\n",Url.c_str());
		unlink(filename.c_str());
		return false;
  }
  return true;
}

CHttpDownloader::CHttpDownloader(){
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
    stats_filepos=1;
    stats_count=1;
}

CHttpDownloader::~CHttpDownloader(){
	curl_easy_cleanup(curl);
	curl=NULL;
}

void CHttpDownloader::setCount(unsigned int count){
	this->stats_count=count;
}

unsigned int CHttpDownloader::getCount(){
	return this->stats_count;
}

unsigned int CHttpDownloader::getStatsPos(){
	return this->stats_filepos;
}

void CHttpDownloader::setStatsPos(unsigned int pos){
	this->stats_filepos=pos;
}


const IDownload* CHttpDownloader::addDownload(const std::string& url, const std::string& filename){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	IDownload* dl=new IDownload(url,filename);
	downloads.push_back(dl);
	return NULL;
}
bool CHttpDownloader::removeDownload(IDownload& download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return true;
}
std::list<IDownload>* CHttpDownloader::search(const std::string& name){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);

	return NULL;
}
void CHttpDownloader::start(IDownload* download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
  	while (!downloads.empty()){
  		this->download(downloads.front()->url,downloads.front()->name);
		downloads.pop_front();
	}
}
