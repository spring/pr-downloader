

#include <curl/curl.h>

#define REPO_MASTER "http://repos.caspring.org/repos.gz"
#define VERSION 0.1
#define USER_AGENT "unitsync-dev" + VERSION


#include <zlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	int written = fwrite(ptr, size, nmemb, stream);
	return written;
}

bool download(char* Url, char* filename){
	CURL *curl;
	CURLcode res;
    printf("Downloading %s to %s\n",Url, filename);

	FILE* fp = fopen(filename ,"wb+");
	if (fp==NULL){
        printf("Could not open %s\n",filename);
		return false;
	}

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if(curl) {
//		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		//    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, (char*)USER_AGENT);


		/* get the first document */
		curl_easy_setopt(curl, CURLOPT_URL, Url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);


		res = curl_easy_perform(curl);

		/* get another document from the same server using the same
		   connection */
		//    curl_easy_setopt(curl, CURLOPT_URL, "http://curl.haxx.se/docs/");
		//    res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
  }
  fclose(fp);
  return true;
}

#define MAX_DATA 5

typedef struct SRepository{
	char* data[MAX_DATA];
	struct SRepository* next;
} SRepository;

/*
parses a repository line like and writes to repository
nota,http://nota.springrts.com,blah,
and fills into data SRepository, size says how many parameters to fill into struct
*/
bool parse_repository_line(char* str, SRepository* repository, int size){
	int pos[MAX_DATA];
	int n=0;
	char* tmp; char* last;
	tmp=str;
	last=str;
	while((tmp!=NULL) && ((unsigned)size<sizeof(pos))){
		tmp=strchr(tmp+1,',');
		if (tmp!=NULL){
			int len=tmp-last;
			repository->data[n]=malloc(len+1);
			strncpy(repository->data[n],last,len);
			repository->data[n][len]=0;
			last=tmp+1;
		}
		n++;
	}
	if (n<size)
		printf("Error parse_repository_line: %s\n",str);
	return (n>=size);
}


/*
	parses a rep master-file and creates a linked list with all reps
*/

SRepository* parse_repository_file(char* filename, int size){
	gzFile fp=gzopen(filename, "r");
	if (fp==Z_NULL){
        printf("Could not open %s\n",filename);
		return NULL;
	}
    char buf[4096];
    SRepository* reps=NULL;
    SRepository* last=NULL;
    while(gzgets(fp, buf, sizeof(buf))!=Z_NULL){
    	SRepository* tmp=malloc(sizeof(SRepository));
    	if (parse_repository_line(buf,tmp, size)){
	   		printf("Found repository %s %s %s\n",tmp->data[0], tmp->data[1], tmp->data[2]);
	   		if (reps==NULL){
	   			reps=tmp;
	   			last=tmp;
			}else{
				last->next=tmp; //append to repository list
				last=tmp;
			}
    	}else{
    		free(tmp);
    	}
    }
    if (last!=NULL)
    	last->next=NULL;
    gzclose(fp);
	return reps;
}

#define TMP_FILE "/tmp/repos.gz"
#define TMP_FILE2 "/tmp/version.gz"
int main(int argc, char **argv){
	char urlbuf[4096];

    download(REPO_MASTER, TMP_FILE);
	SRepository* reps =parse_repository_file(TMP_FILE,4);
	if (reps==NULL){
		printf("Couldn't parse %s\n",TMP_FILE);
		return -1;
	}

	SRepository* tmp;
	tmp=reps;
	while(tmp!=NULL){
		snprintf(urlbuf,sizeof(urlbuf),"%s/versions.gz",tmp->data[1]);
    	download(urlbuf, tmp->data[0]);
		tmp=tmp->next;
	}

	tmp=reps;
	while(tmp!=NULL){
		SRepository* repstmp =parse_repository_file(tmp->data[0],4);
		tmp=tmp->next;
	}


    return 0;
}


