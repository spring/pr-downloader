#include "RepoMaster.h"
#include "FileSystem.h"
#include "HttpDownload.h"
#include "Repo.h"
#include <string>
#include <stdio.h>
#include <zlib.h>
#include "Util.h"

CRepoMaster* CRepoMaster::singleton = NULL;

void CRepoMaster::download(const std::string& name){
	const std::string* tmp=fileSystem->createTempFile();
	tmpFile=*tmp;
	httpDownload->download(name, tmpFile);
	parse();
}

void CRepoMaster::Initialize(std::string& masterurl){
	singleton=new CRepoMaster(masterurl);
}

/*
	parses a rep master-file
*/
void CRepoMaster::parse(){
	gzFile fp=gzopen(tmpFile.c_str(), "r");
	if (fp==Z_NULL){
        printf("Could not open %s\n",tmpFile.c_str());
		return;
	}
    char buf[4096];
    repos.empty();
    while(gzgets(fp, buf, sizeof(buf))!=Z_NULL){
    	std::string tmp=buf;
		std::string* url=getStrByIdx(tmp,',',1);
		if (url->size()>0){ //create new repo from url
			CRepo* repotmp=new CRepo(*url);
			repos.push_back(repotmp);
		}
		delete(url);
    }
    gzclose(fp);
}

void CRepoMaster::updateRepos(){
	std::list<CRepo*>::iterator it;
	for (it = repos.begin();it != repos.end(); ++it) {
		(*it)->download((*it)->getUrl());
	}
}
