#include "RepoMaster.h"
#include "RapidDownloader.h"
#include "../../FileSystem.h"
#include "Repo.h"
#include <string>
#include <stdio.h>
#include <zlib.h>
#include "../../Util.h"

void CRepoMaster::download(const std::string& name)
{
	std::string tmp;
	urlToPath(name,tmp);
	this->path = fileSystem->getSpringDir() + PATH_DELIMITER +"rapid" +PATH_DELIMITER+ tmp;
	fileSystem->createSubdirs(path);
	DEBUG_LINE("%s",name.c_str());
	if (fileSystem->isOlder(path,REPO_MASTER_RECHECK_TIME)) //first try already downloaded file, as repo master file rarely changes
		if (parse()) return;
	IDownload dl(path);
	dl.addMirror(name);
	httpDownload->download(dl);
	parse();
}

CRepoMaster::CRepoMaster(const std::string& url, CRapidDownloader* rapid)
{
	DEBUG_LINE("Added master repo %s", url.c_str());
	this->url=url;
	this->rapid=rapid;
}

bool CRepoMaster::parse()
{
	gzFile fp=gzopen(path.c_str(), "r");
	if (fp==Z_NULL) {
		printf("Could not open %s\n",path.c_str());
		return false;
	}
	char buf[4096];
	repos.empty();
	int i=0;
	while (gzgets(fp, buf, sizeof(buf))!=Z_NULL) {
		std::string tmp=buf;
		std::string url=getStrByIdx(tmp,',',1);
		i++;
		if (url.size()>0) { //create new repo from url
			CRepo repotmp=CRepo(url, rapid);
			repos.push_back(repotmp);
		} else {
			printf("Parse Error %s, Line %d: %s\n",path.c_str(),i,buf);
			return false;
		}
	}
	gzclose(fp);
	//(koshi) both %d and %u throw a type mismatch warning here for me
	printf("Found %d repos in %s\n",repos.size(),path.c_str());
	return true;
}

void CRepoMaster::updateRepos()
{
	DEBUG_LINE("%s","Updating repos...");
	download(url);
	std::list<CRepo>::iterator it;
	for (it = repos.begin(); it != repos.end(); ++it) {
		(*it).download();
	}
}

CRepoMaster::~CRepoMaster()
{

}
