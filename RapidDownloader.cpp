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
	CFileSystem::Shutdown();
}


void CRapidDownloader::addRemoteDsp(CSdp& sdp){
	sdps.push_back(&sdp);
}

std::string toremove;
bool single_digit (CSdp* value){
	std::string tmp=value->getMD5();
	return (tmp.compare(toremove)==0);
}

void CRapidDownloader::removeRemoteDsp(CSdp& sdp){
	toremove=sdp.getMD5();
	sdps.remove_if(single_digit);
}

void CRapidDownloader::list_tag(){
	reloadRepos();
	std::list<CSdp*>::iterator it;
	for(it=sdps.begin();it!=sdps.end();++it){
		CSdp* tmp=*it;
		printf("%s %s\n",tmp->getShortName().c_str(),tmp->getName().c_str());
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
		CSdp* tmp=*it;
		if (tmp->getShortName().compare(modname)==0){
			printf("Found Repository with mod, downloading %s\n", tmp->getMD5().c_str());
			tmp->download();
			return true;
		}
	}
	printf("Couldn't find %s\n", modname.c_str());
	return false;
}

CRapidDownloader::CRapidDownloader(){
	reposLoaded = false;
}
