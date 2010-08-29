
#include "HttpDownload.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include "FileSystem.h"


CHttpDownload* CHttpDownload::singleton = NULL;

/** *
	draw a nice download status-bar
*/
int progress_func(int pos, double TotalToDownload, double NowDownloaded,
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
    printf("%5d/%5d ", pos,httpDownload->getCount());
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

bool CHttpDownload::download(const std::string& Url, const std::string& filename, int pos){
	CURLcode res=CURLE_OK;
    printf("Downloading %s to %s\n",Url.c_str(), filename.c_str());

	FILE* fp = fopen(filename.c_str() ,"wb+");
	if (fp<=NULL){
        printf("Could not open %s\n",filename.c_str());
		return false;
	}
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA , pos);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_URL, Url.c_str());
		res = curl_easy_perform(curl);
	}
	fclose(fp);
	printf("\n"); //new line because of downloadbar
	if (res!=0){
		printf("error downloading %s\n",Url.c_str());
		unlink(filename.c_str());
		return false;
  }
  return true;
}

CHttpDownload::CHttpDownload(){
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
    count=1;
}

CHttpDownload::~CHttpDownload(){
	curl_easy_cleanup(curl);
	curl=NULL;
}

void CHttpDownload::Initialize(){
	singleton=new CHttpDownload();
}

void CHttpDownload::Shutdown(){
	delete(singleton);
	singleton=NULL;
}

void CHttpDownload::setCount(unsigned int count){
	this->count=count;
}

unsigned int CHttpDownload::getCount(){
	return this->count;
}
