#include "RepoMaster.h"
#include "Repo.h"
#include "FileSystem.h"
#include "HttpDownload.h"
#include "RapidDownloader.h"
#include "Sdp.h"
#include "Util.h"
#include <zlib.h>
#include <stdio.h>



void CRepo::download(const std::string& url){
	const std::string* tmp=fileSystem->createTempFile();
	tmpFile=*tmp;
	std::string fullUrl;
	fullUrl=url+"/versions.gz";
	httpDownload->download(fullUrl, tmpFile);
	parse();
}

/*
	parse a repo file (versions.gz)
	a line looks like
	nota:revision:1,52a86b5de454a39db2546017c2e6948d,,NOTA test-1
*/
void CRepo::parse(){
	gzFile fp=gzopen(tmpFile.c_str(), "r");
	if (fp==Z_NULL){
        printf("Could not open %s\n",tmpFile.c_str());
		return;
	}
    char buf[4096];
	sdps.empty();
    while(gzgets(fp, buf, sizeof(buf))!=Z_NULL){
    	for(unsigned int i=0;i<sizeof(buf);i++){
    		if(buf[i]=='\n'){
    			buf[i]=0;
    			break;
    		}
    	}

    	std::string tmp=buf;
		std::string* shortname=getStrByIdx(tmp,',',0);
		std::string* md5=getStrByIdx(tmp,',',1);
		std::string* name=getStrByIdx(tmp,',',3);
		if (shortname->size()>0){ //create new repo from url
			CSdp* sdptmp=new CSdp(*shortname, *md5, *name, repourl);
			rapidDownloader->addRemoteDsp(*sdptmp);
		}
    }
    gzclose(fp);
}
