#include "RepoMaster.h"
#include "RapidDownloader.h"
#include "../../FileSystem.h"
#include "Repo.h"
#include <string>
#include <stdio.h>
#include <zlib.h>
#include "../../Util.h"

void CRepoMaster::download(const std::string& name){
	std::string tmp;
	urlToPath(name,tmp);
	this->path = fileSystem->getSpringDir() + PATH_DELIMITER +"rapid" +PATH_DELIMITER+ tmp;
	fileSystem->createSubdirs(path);
	printf("%s %s:%d '%s'\n",__FILE__, __FUNCTION__ ,__LINE__, path.c_str());
	if (fileSystem->isOlder(path,REPO_MASTER_RECHECK_TIME)) //first try already downloaded file, as repo master file rarely changes
		if (parse()) return;
	httpDownload->addDownload(name, path);
	httpDownload->start();
	parse();
}

CRepoMaster::CRepoMaster(std::string& url){
	this->url=url;
}

bool CRepoMaster::parse(){
	gzFile fp=gzopen(path.c_str(), "r");
	if (fp==Z_NULL){
        printf("Could not open %s\n",path.c_str());
		return false;
	}
	printf("parsing %s\n",path.c_str());
    char buf[4096];
    repos.empty();
    int i=0;
    while(gzgets(fp, buf, sizeof(buf))!=Z_NULL){
    	std::string tmp=buf;
		std::string url=getStrByIdx(tmp,',',1);
		i++;
		if (url.size()>0){ //create new repo from url
			CRepo* repotmp=new CRepo(url);
			repos.push_back(repotmp);
		}else{
			printf("Parse Error %s, Line %d: %s\n",path.c_str(),i,buf);
			return false;
		}
    }
    gzclose(fp);
    return true;
}

void CRepoMaster::updateRepos(){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	std::list<CRepo*>::iterator it;
	for (it = repos.begin();it != repos.end(); ++it) {
		(*it)->download();
	}
}

CRepoMaster::~CRepoMaster(){
}
