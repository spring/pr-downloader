/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "RepoMaster.h"
#include "RapidDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Repo.h"
#include "Logger.h"
#include "Downloader/IDownloader.h"

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
	//first try already downloaded file, as repo master file rarely changes
	if ((fileSystem->fileExists(path)) && (fileSystem->isOlder(path,REPO_MASTER_RECHECK_TIME)) && parse())
		return;
	IDownload dl(path);
	dl.addMirror(name);
	httpDownload->download(&dl);
	parse();
}

CRepoMaster::CRepoMaster(const std::string& url, CRapidDownloader* rapid):
	url(url),
	rapid(rapid)
{
	LOG_DEBUG("Added master repo %s", url.c_str());
}

bool CRepoMaster::parse()
{
	gzFile fp=gzopen(path.c_str(), "r");
	if (fp==Z_NULL) {
		LOG_ERROR("Could not open %s",path.c_str());
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
			LOG_ERROR("Parse Error %s, Line %d: %s",path.c_str(),i,buf);
			return false;
		}
	}
	gzclose(fp);
	LOG_INFO("Found %d repos in %s",repos.size(),path.c_str());
	return true;
}

void CRepoMaster::updateRepos()
{
	LOG_DEBUG("%s","Updating repos...");
	download(url);
	std::list<CRepo>::iterator it;
	std::list<IDownload*> dls;
	for (it = repos.begin(); it != repos.end(); ++it) {
		IDownload* dl = new IDownload();
		if ((*it).getDownload(*dl)) {
			dls.push_back(dl);
		} else {
			delete dl;
		}
	}
	httpDownload->download(dls);
	for (it = repos.begin(); it != repos.end(); ++it) {
		(*it).parse();
	}
	IDownloader::freeResult(dls);
}

CRepoMaster::~CRepoMaster()
{

}
