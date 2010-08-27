
#include "HttpDownload.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>

CHttpDownload* CHttpDownload::singleton = NULL;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	int written = fwrite(ptr, size, nmemb, stream);
	return written;
}

bool CHttpDownload::download(const std::string& Url,const std::string& filename){
	CURLcode res=CURLE_OK;
    printf("Downloading %s to %s\n",Url.c_str(), filename.c_str());

	FILE* fp = fopen(filename.c_str() ,"wb+");
	if (fp==NULL){
        printf("Could not open %s\n",filename.c_str());
		return false;
	}
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_URL, Url.c_str());
		res = curl_easy_perform(curl);
  }
  fclose(fp);
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
}

CHttpDownload::~CHttpDownload(){
	curl_easy_cleanup(curl);
}

void CHttpDownload::Initialize(){
	singleton=new CHttpDownload();
}
