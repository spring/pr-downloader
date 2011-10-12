#include "RepoMaster.h"
#include "RapidDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Repo.h"

#include <string>
#include <stdio.h>
#include <zlib.h>

void CRepoMaster::download(const std::string& name)
{
	std::string tmp;
	urlToPath(name,tmp);
	this->path = fileSystem->getSpringDir() + PATH_DELIMITER +"rapid" +PATH_DELIMITER+ tmp;
	fileSystem->createSubdirs(path);
	LOG_DEBUG("%s",name.c_str());
	if (fileSystem->isOlder(path,REPO_MASTER_RECHECK_TIME)) //first try already downloaded file, as repo master file rarely changes
		if (parse()) return;
	IDownload dl(path);
	dl.addMirror(name);
	httpDownload->download(dl);
	parse();
}

CRepoMaster::CRepoMaster(const std::string& url, CRapidDownloader* rapid)
{
	LOG_DEBUG("Added master repo %s", url.c_str());
	this->url=url;
	this->rapid=rapid;
}

bool CRepoMaster::parse()
{
	gzFile fp=gzopen(path.c_str(), "r");
	if (fp==Z_NULL) {
		LOG_ERROR("Could not open %s\n",path.c_str());
		return false;
	}
	char buf[IO_BUF_SIZE];
	repos.clear();
	int i=0;
	while (gzgets(fp, buf, sizeof(buf))!=Z_NULL) {
		std::string tmp=buf;
		std::string url=getStrByIdx(tmp,',',1);
		i++;
		if (url.size()>0) { //create new repo from url
			CRepo repotmp=CRepo(url, rapid);
			repos.push_back(repotmp);
		} else {
			LOG_ERROR("Parse Error %s, Line %d: %s\n",path.c_str(),i,buf);
			return false;
		}
	}
	gzclose(fp);
	LOG_INFO("Found %d repos in %s\n",repos.size(),path.c_str());
	return true;
}

void CRepoMaster::updateRepos()
{
	LOG_DEBUG("%s","Updating repos...");
	download(url);
	std::list<CRepo>::iterator it;
	for (it = repos.begin(); it != repos.end(); ++it) {
		(*it).download();
	}
}

CRepoMaster::~CRepoMaster()
{

}
