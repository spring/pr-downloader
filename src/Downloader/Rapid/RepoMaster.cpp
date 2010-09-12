#include "RepoMaster.h"
#include "../../FileSystem.h"
#include "Repo.h"
#include <string>
#include <stdio.h>
#include <zlib.h>
#include "../../Util.h"
#include "../IDownloader.h"

void CRepoMaster::download(const std::string& name){
	tmpFile=fileSystem->createTempFile();
	httpDownload->addDownload(name, tmpFile);
	httpDownload->start();
	parse();
}

CRepoMaster::CRepoMaster(std::string& url){
	this->url=url;
}

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
		std::string url=getStrByIdx(tmp,',',1);
		if (url.size()>0){ //create new repo from url
			CRepo* repotmp=new CRepo(url);
			repos.push_back(repotmp);
		}
    }
    gzclose(fp);
}

void CRepoMaster::updateRepos(){
	std::list<CRepo*>::iterator it;
	for (it = repos.begin();it != repos.end(); ++it) {
		(*it)->download();
	}
}

CRepoMaster::~CRepoMaster(){
}
