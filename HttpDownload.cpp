
#include "HttpDownload.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include "FileSystem.h"
#include <zlib.h>
#include "Util.h"


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

bool initianlized;
unsigned long written_tofile; //bytes written to current file in total
int filepos; //current file # we are downloading
FILE*f; //current file
std::string filename;
std::list<CFileSystem::FileData*>::iterator it;
std::list<CFileSystem::FileData*>* globalFiles;
bool skipped;

static size_t write_streamed_data(void* buf, size_t size, size_t nmemb, FILE *stream) {
	unsigned int towrite=size*nmemb; //count of bytes to write this pass
	char* pos; //bytes written in this pass
	unsigned int bytes=0;
	pos=(char*)buf;
	if(!initianlized){ //initialize
		it=globalFiles->begin();
		initianlized=true;
		f=NULL;
		filename="";
		written_tofile=0;
	}
	while(bytes<=towrite){ //repeat until all avaiable data is written
		while( (f==NULL) && ( it != globalFiles->end())){//get next file
			if ((*it)->download==true){
				//now open new file
				filename=fileSystem->getPoolFileName(*it);
				f=fopen(filename.c_str(),"wb+");
				skipped=false;
				filepos++; //inc files downloading
				if (f==NULL){
					printf("\n ------------------------------- Error opening %s\n",filename.c_str());
					return -1;
				}
			}else{
				it++;
			}
		}

		if (f!=NULL){ //file is already open, write or close it
			if (written_tofile<(*it)->size){//if so, then write bytes left
				unsigned int left=(*it)->size - written_tofile;
				if (towrite<left)
					left=towrite;

				if (!skipped){ //first write to file, skip the 4 length bytes
					bytes+=4;
					*pos+=4;
					skipped=true;
				}


				int res=fwrite(pos,1,left,f);
				if(res<=0){
					printf("\n -------------------------- Error in fwrite\n");
					return -1;
				}

				written_tofile+=res;
				bytes+=res;
			}
			if (written_tofile>=(*it)->size){ //file end reached
				fclose(f);
				if (fileSystem->fileIsValid(*it,filename)){//damaged file downloaded, abort!
					printf("\nDamaged File %s %d\n",filename.c_str(), (*it)->size);
					return -1;
				}
				filename="";
				written_tofile=0;
				++it;
				f=NULL;
			}
		}
	}
    fflush(stdout);
	return nmemb*size;
}

/**
	download files streamed
	streamer.cgi works as follows:
	* The client does a POST to /streamer.cgi?<hex>
	  Where hex = the name of the .sdp
	* The client then sends a gzipped bitarray representing the files
	  it wishes to download. Bitarray is formated in the obvious way,
	  an array of characters where each file in the sdp is represented
	  by the (index mod 8) bit (shifted left) of the (index div 8) byte
	  of the array.
	* streamer.cgi then responds with <big endian encoded int32 length>
	  <data of gzipped pool file> for all files requested. Files in the
	  pool are also gzipped, so there is no need to decompress unless
	  you wish to verify integrity.
	* streamer.cgi also sets the Content-Length header in the reply so
	  you can implement a proper progress bar.

T 192.168.1.2:33202 -> 94.23.170.70:80 [AP]
POST /streamer.cgi?652e5bb5028ff4d2fc7fe43a952668a7 HTTP/1.1..Accept-Encodi
ng: identity..Content-Length: 29..Host: packages.springrts.com..Content-Typ
e: application/x-www-form-urlencoded..Connection: close..User-Agent: Python
-urllib/2.6....
##
T 192.168.1.2:33202 -> 94.23.170.70:80 [AP]
......zL..c`..`d.....K.n/....
*/
void CHttpDownload::downloadStream(std::string url,std::list<CFileSystem::FileData*>& files){
	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();
	initianlized=false;
	filepos=1;
	if(curl) {
		printf("%s\n",url.c_str());

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

		std::list<CFileSystem::FileData*>::iterator it;
		int  buflen=files.size()/8;
		if (files.size()%8!=0)
			buflen++;
		char* buf=(char*)malloc(buflen); //FIXME: compress blockwise and not all at once
		memset(buf,0,buflen);
		int destlen=files.size()*2;
		printf("%d %d %d\n",(int)files.size(),buflen,destlen);
		int i=0;
		for(it=files.begin();it!=files.end();it++){
			if ((*it)->download==true)
				buf[i/8] = buf[i/8] + (1<<(i%8));
			i++;
		}
		char* dest=(char*)malloc(destlen);

		gzip_str(buf,buflen,dest,&destlen);
/*
		FILE* f;
		f=fopen("request","w");
		fwrite(buf, buflen,1,f);
		fclose(f);

		f=fopen("request.gz","w");
		fwrite(dest, destlen,1,f);
		fclose(f);
*/
		free(buf);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_streamed_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &files);
		globalFiles=&files;
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dest);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,destlen);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA , filepos);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);

		res = curl_easy_perform(curl);
		if (res!=CURLE_OK){
			printf("%s\n",curl_easy_strerror(res));
		}
		free(dest);
		/* always cleanup */
		curl_easy_cleanup(curl);
  }
}
