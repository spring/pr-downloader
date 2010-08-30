#include "RapidDownloader.h"
#include "FileSystem.h"
#include "Util.h"
#include "HttpDownload.h"
#include "Sdp.h"
#include <stdio.h>
#include <string>
#include <string.h>
#include <list>
#include "RepoMaster.h"


CRapidDownloader* CRapidDownloader::singleton = NULL;

void CRapidDownloader::Initialize(){
	singleton=new CRapidDownloader();
	CHttpDownload::Initialize();
	CFileSystem::Initialize();
	std::string url(REPO_MASTER);
	CRepoMaster::Initialize(url);
}

void CRapidDownloader::Shutdown(){
	CRepoMaster::Shutdown();
	CFileSystem::Shutdown();
	CHttpDownload::Shutdown();
	delete(singleton);
	singleton=NULL;
}


void CRapidDownloader::addRemoteDsp(CSdp& sdp){
	sdps.push_back(&sdp);
}

static bool list_compare(CSdp* first ,CSdp*  second){
	std::string name1;
	std::string name2;
	name1.clear();
	name2.clear();

	name1=(first->getShortName());
	name2=(second->getShortName());
	unsigned int len;
	len=name1.size();
	if (len<name2.size())
		len=name2.size();
	for(unsigned int i=0;i<len;i++){
		if (tolower(name1[i])<tolower(name2[i])){
			return true;
		}
	}
	return false;
}

void CRapidDownloader::list_tag(){
	reloadRepos();
	sdps.sort(list_compare);
	std::list<CSdp*>::iterator it;
	for(it=sdps.begin();it!=sdps.end();++it){
		printf("%-40s%s\n",(*it)->getShortName().c_str(),(*it)->getName().c_str());
	}
}

bool CRapidDownloader::reloadRepos(){
	if (reposLoaded)
		return true;
	std::string url(REPO_MASTER);
	repoMaster->download(url);
	repoMaster->updateRepos();
	reposLoaded=true;
	return true;
}

bool CRapidDownloader::download_tag(const std::string& modname){
	reloadRepos();
	std::list<CSdp*>::iterator it;
	for(it=sdps.begin();it!=sdps.end();++it){
		if ((*it)->getShortName().compare(modname)==0){
			printf("Found Repository, downloading %s\n", (*it)->getName().c_str());
			(*it)->download();
			return true;
		}
	}
	printf("Couldn't find %s\n", modname.c_str());
	return false;
}

CRapidDownloader::CRapidDownloader(){
	reposLoaded = false;
}
