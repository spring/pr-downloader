
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

CHttpDownloader::CHttpDownloader(){
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

char buf[4096];
int pos;

static size_t write_mem(void *ptr, size_t size, size_t nmemb, void *data){
	size_t realsize = size * nmemb;
	if (pos+realsize>sizeof(buf))
		return 0;
	memcpy(&buf[pos],ptr,realsize);
	pos=pos+realsize;
	return realsize;
}



std::list<IDownload>* CHttpDownloader::realSearch(const std::string& name, IDownload::category cat){
	DEBUG_LINE(name.c_str());
	pos=0;
	memset(buf,0,sizeof(buf));
	CURL *curl_handle;
	curl_handle=curl_easy_init();
	if (curl_handle){
		std::string url;
		std::string filename;
		filename=fileSystem->getSpringDir();
		filename+=PATH_DELIMITER;
		switch (cat){
			case IDownload::CAT_MODS:
				url="http://www.springfiles.com/checkmirror.php?c=mods&q=";
				filename+="mods";
			break;
			default:
				url="http://www.springfiles.com/checkmirror.php?c=maps&q=";
				filename+="maps";
			break;
		}
		filename+=PATH_DELIMITER;
		filename+=name;
		url+=name;

		curl_easy_setopt(curl_handle, CURLOPT_URL,url.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_mem);
		curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, true);
		curl_easy_perform(curl_handle);

		std::list<IDownload>* res;
		res=new std::list<IDownload>;
		int start=-1;
		for(unsigned int i=0;i<sizeof(buf);i++){ //extract urls of result in buf
			if (buf[i]=='\'' ){
				if (start==-1){
					start=i;
				}else{
					buf[i]=0;
					std::string url;
					url.assign(&buf[start+1]);
					IDownload tmp(url,filename, cat);
					res->push_back(tmp);
					start=-1;
				}
			}
		}
		return res;
	}
	return NULL;

}

std::list<IDownload>* CHttpDownloader::search(const std::string& name, IDownload::category cat){
	DEBUG_LINE("");
	std::list<IDownload>* res;
	res=realSearch(name+".sd7", cat);
	if (!res->empty()) return res;
	res=realSearch(name+".sdz", cat);
	if (!res->empty()) return res;
	return NULL;
}

bool CHttpDownloader::download(IDownload& download){
	DEBUG_LINE("");

	CURLcode res=CURLE_OK;
    printf("CHttpDownloader::download %s to %s\n",download.url.c_str(), download.name.c_str());

	if(!curl) {
		printf("Error initializing curl");
		return false;
	}
	FILE* fp = fopen(download.name.c_str() ,"wb+");
	if (fp==NULL){
        printf("CHttpDownloader:: Could not open %s\n",download.name.c_str());
		return false;
	}
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA ,this);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_URL, download.url.c_str());
	res = curl_easy_perform(curl);
	fclose(fp);
	printf("\n"); //new line because of downloadbar
	if (res!=0){
		printf("error downloading %s\n",download.url.c_str());
		unlink(download.name.c_str());
		return false;
	}
	return true;
}
