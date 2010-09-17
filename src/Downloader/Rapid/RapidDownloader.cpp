#include "RapidDownloader.h"
#include "../../FileSystem.h"
#include "../../Util.h"
#include "Sdp.h"
#include <stdio.h>
#include <string>
#include <string.h>
#include <list>
#include "RepoMaster.h"


CRapidDownloader::CRapidDownloader(){
	std::string url(REPO_MASTER);
	this->repoMaster=new CRepoMaster(url);
	reposLoaded = false;
	sdps.clear();
}

CRapidDownloader::~CRapidDownloader(){
	delete(repoMaster);
}


void CRapidDownloader::addRemoteDsp(CSdp* sdp){
	sdps.push_back(sdp);
}

/**
	helper function for sort
*/
static bool list_compare(CSdp* first ,CSdp*  second){
	std::string name1;
	std::string name2;
	name1.clear();
	name2.clear();
printf("%s\n",first->getShortName().c_str());
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
/**
	update all repos from the web
*/
bool CRapidDownloader::reloadRepos(){
	if (reposLoaded)
		return true;
	std::string url(REPO_MASTER);
	this->repoMaster->download(url);
	repoMaster->updateRepos();
	reposLoaded=true;
	return true;
}
/**
	download by tag, for example "ba:stable"
*/
bool CRapidDownloader::download_tag(const std::string& modname){
	reloadRepos();
	std::list<CSdp*>::iterator it;
	for(it=sdps.begin();it!=sdps.end();++it){
		if ((*it)->getShortName().compare(modname)==0){
			printf("Found Repository, downloading %s\n", (*it)->getName().c_str());
			(*it)->download();
			if ((*it)->getDepends().length()>0){
				download_name((*it)->getDepends());
			}
			return true;
		}
	}
	printf("Couldn't find %s\n", modname.c_str());
	return false;
}

/**
	download by name, for example "Complete Annihilation revision 1234"
*/
bool CRapidDownloader::download_name(const std::string& longname, int reccounter){
	std::list<CSdp*>::iterator it;
	if (reccounter>10)
		return false;
	for(it=sdps.begin();it!=sdps.end();++it){
		if ((*it)->getName().compare(longname)==0){
			printf("Found Depends, downloading %s\n", (*it)->getName().c_str());
			(*it)->download();
			if ((*it)->getDepends().length()>0){
				download_name((*it)->getDepends(),reccounter+1); //FIXME: this could be an infinite loop
			}
			return true;
		}
	}
	return false;
}


const IDownload* CRapidDownloader::addDownload(const std::string& url, const std::string& filename){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	download_tag(url);
	return NULL;
}

bool CRapidDownloader::removeDownload(IDownload& download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return true;
}
/**
	search for a mod
*/
std::list<IDownload>* CRapidDownloader::search(const std::string& name){
	printf("%s %s:%d  '%s'\n",__FILE__, __FUNCTION__ ,__LINE__, name.c_str() );
	reloadRepos();
	std::list<IDownload>*tmp;
	tmp=new std::list<IDownload>;

	sdps.sort(list_compare);
	std::list<CSdp*>::iterator it;
	printf("blaa\n");
	for(it=this->sdps.begin();it!=this->sdps.end();++it){
		IDownload* dl;
		dl=new IDownload((*it)->getShortName().c_str(),(*it)->getName().c_str());
		printf("test %s\n",name.c_str());
		if (match_download_name(dl->name,name))
			tmp->push_back(*dl);
	}
	return tmp;
}
/**
	start a download
*/
void CRapidDownloader::start(IDownload* download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	if (download==NULL){
  		while (!downloads.empty()){
	  		this->download_tag(downloads.front()->url);
			downloads.pop_front();
		}
	}else{
		this->download_tag(download->url);
	}
}

