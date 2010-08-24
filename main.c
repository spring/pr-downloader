

#include <curl/curl.h>

#define REPO_MASTER "http://repos.caspring.org/repos.gz"
#define VERSION 0.1
#define USER_AGENT "unitsync-dev" + VERSION


#include <zlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	int written = fwrite(ptr, size, nmemb, stream);
	return written;
}

bool download(char* Url, char* filename){
	CURL *curl;
	CURLcode res=0;
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
		printf("error downloading");
		unlink(filename);
		return false;
  }
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

//FIXME: not portable
typedef struct SMod{
	u_int8_t namelen;//
	char name[256];
	unsigned char md5[16]; //16
	unsigned int crc32[4]; //4
	unsigned int size[4]; //4
	struct SMod* next;
}SMod;

bool getUrl(SMod* mod, char* mirror, char* buf, int size){
	int i;
	char md5[32];
	for (i = 0; i < 16; i++){
		sprintf(md5+i*2, "%02x", mod->md5[i]);
	}
	int res=snprintf(buf,size,"%spool/%c%c/%s.gz",mirror,md5[0],md5[1],md5+2);
	return true;

}

SMod* parse_binary_file(char* filename){
	gzFile fp=gzopen(filename, "r");
	printf("parse_binary: %s\n",filename);
	if (fp==Z_NULL){
        printf("Could not open %s\n",filename);
		return NULL;
	}
	SMod* res=NULL;
	SMod* tmp=NULL;
	while(!gzeof(fp)){
		if(res==NULL){
			res=malloc(sizeof(SMod));
			tmp=res;
		}else{
			tmp->next=malloc(sizeof(SMod));
			tmp=tmp->next;
		}
		gzread(fp,&tmp->namelen, 1);
		gzread(fp,&tmp->name, tmp->namelen);
		tmp->name[tmp->namelen]=0;
		gzread(fp,&tmp->md5, 24);
	}
	if(tmp!=NULL)
		tmp->next=NULL;
	return res;
}
/*
	download the .sdp + all associated files
	package is the md5sum
	spring is for example ~/.spring
*/
bool download_revision(char* mirror, char* package, char* springwritedir){
	char urlbuf[1024];
	char pathbuf[1024];

	snprintf(urlbuf,sizeof(urlbuf),"%s/packages/%s.sdp",mirror,package);
	snprintf(pathbuf,sizeof(pathbuf),"%s/packages/%s.sdp",springwritedir,package);

/*	if (!download(urlbuf, pathbuf))
		return false;
*/
	SMod* mod=parse_binary_file(pathbuf);
	while(mod!=NULL){
		getUrl(mod,mirror,urlbuf,sizeof(urlbuf));
		printf("%s\n",urlbuf);
		mod=mod->next;
	}

	return true;
}


#define TMP_FILE "/tmp/repos.gz"
#define TMP_FILE2 "/tmp/version.gz"
int main(int argc, char **argv){
/*	char urlbuf[4096];

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
	SRepository* mods=NULL;

	while(tmp!=NULL){
		SRepository* repstmp = parse_repository_file(tmp->data[0],4);
		if (repstmp!=NULL){
			if (mods==NULL){ //set first element
				mods = repstmp;
			}else{ //append
				SRepository* modstmp = mods;
				while(modstmp->next!=NULL)
					modstmp=modstmp->next;
				modstmp->next=repstmp;
			}
		}
		tmp=tmp->next;
	}

*/
	SRepository* mods=parse_repository_file("ca",3);

	SRepository* modstmp=mods;
	int i=0;
	while(modstmp!=NULL){
		printf("%d %s %s %s\n",i,modstmp->data[0], modstmp->data[1], modstmp->data[2]);
		i++;
		modstmp=modstmp->next;
	}

	download_revision("http://nota.springrts.com/","52a86b5de454a39db2546017c2e6948d","/home/matze/.spring");


    return 0;
}


