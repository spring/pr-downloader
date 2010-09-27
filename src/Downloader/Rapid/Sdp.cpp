#include "RapidDownloader.h"
#include "RepoMaster.h"
#include "Sdp.h"
#include "../../FileSystem.h"
#include "../../Util.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>

void CSdp::download(){
	DEBUG_LINE("");
	if(downloaded) //allow download only once of the same sdp
		return;
	filename=fileSystem->getSpringDir() + PATH_DELIMITER+"packages"+PATH_DELIMITER;
	if (!fileSystem->directoryExists(filename)){
		fileSystem->createSubdirs(filename);
	}
	int count=0;
	filename  += this->md5 + ".sdp";
	CFileSystem::FileData tmp;
	md5AtoI(md5,tmp.md5);
	std::list<CFileSystem::FileData*> files;

	if (!fileSystem->parseSdp(filename,files)){ //file isn't avaiable, download it
		httpDownload->addDownload(url + "/packages/" + md5 + ".sdp", filename);
		httpDownload->start();
		fileSystem->parseSdp(filename,files); //parse downloaded file
	}

	std::list<CFileSystem::FileData*>::iterator it;
/*	CHttpDownload* tmp=httpDownload; //FIXME: extend interface?
	tmp->setCount(files.size());
*/
	int i=0;
	it=files.begin();
	while(it!=files.end()){
		i++;
		std::string tmpmd5="";
		md5ItoA((*it)->md5, tmpmd5);
		std::string filename=tmpmd5.substr(2);
		filename.append(".gz");
		std::string path("/pool/");
		path += tmpmd5.at(0);
		path += tmpmd5.at(1);
		path += "/";

		std::string file=fileSystem->getSpringDir() + path + filename; //absolute filename

		if (!fileSystem->directoryExists(fileSystem->getSpringDir()+path)){
			fileSystem->createSubdirs(fileSystem->getSpringDir()+path);
		}
		if(!fileSystem->fileIsValid(*it,file)){ //add invalid files to download list
			count++;
			(*it)->download=true;
		}else{
			(*it)->download=false;
		}
		if (i%10==0)
			printf("\r%d/%d checked",i,(int)files.size());
		it++;
	}
	printf("\r%d/%d need to download %d files\n",i,(unsigned int)files.size(),count);
	if (count>0){
//FIXME	httpDownload->setCount(count);
		downloadStream(this->url+"/streamer.cgi?"+this->md5,files);
		files.clear();
		printf("Sucessfully downloaded %d files: %s %s\n",count,shortname.c_str(),name.c_str());
	}else
		printf("Already downloaded: %s\n", shortname.c_str());

	downloaded=true;
}

/**
	write the data received from curl to the rapid pool.

	the filename is read from the sdp-list (created at request start)
	filesize is read from the http-data received (could overlap!)
*/
static size_t write_streamed_data(const void* tmp, size_t size, size_t nmemb,CSdp *sdp) {
	char buf[CURL_MAX_WRITE_SIZE];
	memcpy(&buf,tmp,CURL_MAX_WRITE_SIZE);
	if(!sdp->downlooadInitialized){
		sdp->list_it=sdp->globalFiles->begin();
		sdp->downlooadInitialized=true;
		sdp->file_handle=NULL;
		sdp->file_name="";
		sdp->skipped=0;
	}
	char* buf_start=(char*)&buf;
	const char* buf_end=buf_start + size*nmemb;
	char* buf_pos=buf_start;

	while(buf_pos<buf_end){ //all bytes written?
		if (sdp->file_handle==NULL){ //no open file, create one
			while( (!(*sdp->list_it)->download==true) && (sdp->list_it!=sdp->globalFiles->end())){ //get file
				sdp->list_it++;
			}
			sdp->file_name=fileSystem->getPoolFileName(*sdp->list_it);
			sdp->file_handle=fopen(sdp->file_name.c_str(),"wb");
//FIXME		sdp->setStatsPos(sdp->getStatsPos()+1);
			if (sdp->file_handle==NULL){
				printf("couldn't open %s\n",(*sdp->list_it)->name.c_str());
				return -1;
			}
			//here comes the init new file stuff
			sdp->file_pos=0;
		}
		if (sdp->file_handle!=NULL){
			if((sdp->skipped>0)&&(sdp->skipped<4)){
//				printf("difficulty %d\n",skipped);
			}
			if (sdp->skipped<4){ // check if we skipped all 4 bytes, if not so, skip them
				int toskip=intmin(buf_end-buf_pos,4-sdp->skipped); //calculate bytes we can skip, could overlap received bufs
				for(int i=0;i<toskip;i++) //copy bufs avaiable
					sdp->cursize_buf[i]=buf_pos[i];
//				printf("toskip: %d skipped: %d\n",toskip,skipped);
				sdp->skipped=toskip+sdp->skipped;
				buf_pos=buf_pos+sdp->skipped;
				if (sdp->skipped==4){
					(*sdp->list_it)->compsize=parse_int32(sdp->cursize_buf);
				}
			}
			if (sdp->skipped==4){
				int towrite=intmin ((*sdp->list_it)->compsize-sdp->file_pos ,  //minimum of bytes to write left in file and bytes to write left in buf
					buf_end-buf_pos);
//				printf("%s %d %ld %ld %ld %d %d %d %d %d\n",file_name.c_str(), (*list_it)->compsize, buf_pos,buf_end, buf_start, towrite, size, nmemb , skipped, file_pos);
				int res=0;
				if (towrite>0){
					res=fwrite(buf_pos,1,towrite,sdp->file_handle);
					if (res!=towrite){
						printf("fwrite didn't write all\n");
					}
					if(res<=0){
						printf("\nwrote error: %d\n", res);
						return -1;
					}
				}

				buf_pos=buf_pos+res;
				sdp->file_pos+=res;
				if (sdp->file_pos>=(*sdp->list_it)->compsize){ //file finished -> next file
					fclose(sdp->file_handle);
					if (!fileSystem->fileIsValid(*sdp->list_it,sdp->file_name.c_str())){
						printf("File is broken?!: %s\n",sdp->file_name.c_str());
						return -1;
					}
					sdp->file_handle=NULL;
					sdp->list_it++;
					sdp->file_pos=0;
					sdp->skipped=0;
				}
			}
		}
	}
	return buf_pos-buf_start;

}

/** *
	draw a nice download status-bar
*/
int progress_func(CSdp& csdp, double TotalToDownload, double NowDownloaded,
                    double TotalToUpload, double NowUploaded){

	(void)csdp;(void)TotalToUpload;(void)NowUploaded; //remove unused warning
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
//    printf("%5d/%5d ", ptr->getStatsPos(),ptr->getCount());
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

void CSdp::downloadStream(std::string url,std::list<CFileSystem::FileData*>& files){
	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();
	if(curl) {
		printf("Downloading stream: %s\n",url.c_str());

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

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

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_streamed_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		globalFiles=&files;
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dest);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,destlen);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);

		res = curl_easy_perform(curl);
		printf("\n"); //new line because of progressbar
		if (res!=CURLE_OK){
			printf("%s\n",curl_easy_strerror(res));
		}
		free(dest);
		/* always cleanup */
		curl_easy_cleanup(curl);
  }
}
