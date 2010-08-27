
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
	CURL *curl;
	CURLcode res=CURLE_OK;
    printf("Downloading %s to %s\n",Url.c_str(), filename.c_str());

	FILE* fp = fopen(filename.c_str() ,"wb+");
	if (fp==NULL){
        printf("Could not open %s\n",filename.c_str());
		return false;
	}

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if(curl) {
//		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		//    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, (char*)USER_AGENT);


		/* get the first document */
		curl_easy_setopt(curl, CURLOPT_URL, Url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

		res = curl_easy_perform(curl);

		/* get another document from the same server using the same
		   connection */
		//    curl_easy_setopt(curl, CURLOPT_URL, "http://curl.haxx.se/docs/");
		//    res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
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

}

void CHttpDownload::Initialize(){
	singleton=new CHttpDownload();
}
