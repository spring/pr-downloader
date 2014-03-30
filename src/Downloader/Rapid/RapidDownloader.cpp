/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "RapidDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Logger.h"
#include "Repo.h"
#include "Sdp.h"

#include <stdio.h>
#include <string>
#include <string.h>
#include <list>
#include <zlib.h>

#ifndef WIN32
#include <regex.h>
#endif


CRapidDownloader::CRapidDownloader()
{
	reposLoaded = false;
	sdps.clear();
	setMasterUrl(REPO_MASTER);
}

CRapidDownloader::~CRapidDownloader()
{
	sdps.clear();
}


void CRapidDownloader::addRemoteDsp(CSdp& sdp)
{
	sdps.push_back(sdp);
}


bool CRapidDownloader::list_compare(CSdp& first ,CSdp& second)
{
	std::string name1;
	std::string name2;
	name1.clear();
	name2.clear();
	name1=(first.getShortName());
	name2=(second.getShortName());
	unsigned int len;
	len=std::min(name1.size(), name2.size());
	for (unsigned int i=0; i<len; i++) {
		if (tolower(name1[i])<tolower(name2[i])) {
			return true;
		}
	}
	return false;
}

bool CRapidDownloader::reloadRepos()
{
	if (reposLoaded)
		return true;
	updateRepos();
	reposLoaded=true;
	return true;
}


bool CRapidDownloader::download_name(IDownload* download, int reccounter,std::string name)
{
	LOG_DEBUG("%s %s",name.c_str(),download->name.c_str());
	std::list<CSdp>::iterator it;
	if (reccounter>10)
		return false;
	LOG_DEBUG("Using rapid");
	for (it=sdps.begin(); it!=sdps.end(); ++it) {
		if (match_download_name((*it).getName(),name.length() == 0 ? download->name : name )) {

			LOG_DOWNLOAD((it)->getName().c_str() );
			if (!(*it).download(download))
				return false;
			if ((*it).getDepends().length()>0) {
				if (!download_name(download,reccounter+1,(*it).getDepends()))
					return false;
			}
			return true;
		}
	}
	return false;
}



bool CRapidDownloader::search(std::list<IDownload*>& result, const std::string& name, IDownload::category cat)
{
	LOG_DEBUG("%s",name.c_str());
	reloadRepos();
	sdps.sort(list_compare);
	std::list<CSdp>::iterator it;
	for (it=sdps.begin(); it!=sdps.end(); ++it) {
		if (match_download_name((*it).getShortName(),name)
		    || (match_download_name((*it).getName(),name))) {
			IDownload* dl=new IDownload((*it).getName().c_str(), name, cat, IDownload::TYP_RAPID);
			dl->addMirror((*it).getShortName().c_str());
			result.push_back(dl);
		}
	}
	return true;
}

bool CRapidDownloader::download(IDownload* download,int max_parallel)
{
	LOG_DEBUG("%s",download->name.c_str());
	if (download->dltype != IDownload::TYP_RAPID) { //skip non-rapid downloads
		LOG_DEBUG("skipping non rapid-dl");
		return true;
	}
	reloadRepos();
	return download_name(download,0);
}

bool CRapidDownloader::match_download_name(const std::string &str1,const std::string& str2)
{
	if (str2=="") return true;
	if (str1==str2) return true;
	if (str2=="*")	 return true;
	//FIXME: add regex support for win32
	/*
	#ifndef WIN32
		regex_t regex;
		if (regcomp(&regex, str2.c_str(), 0)==0) {
			int res=regexec(&regex, str1.c_str(),0, NULL, 0 );
			regfree(&regex);
			if (res==0) {
				return true;
			}
		}
	#endif
	*/
	return false;
}

void CRapidDownloader::setMasterUrl(const std::string& url)
{
	this->url=url;
	reposLoaded = false;
}


void CRapidDownloader::download(const std::string& name)
{
	std::string tmp;
	if (!urlToPath(name,tmp)){
		LOG_ERROR("Invalid path: %s", tmp.c_str());
		return;
	}
	path = fileSystem->getSpringDir() + PATH_DELIMITER +"rapid" +PATH_DELIMITER+ tmp;
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

bool CRapidDownloader::parse()
{
	FILE* f = fileSystem->propen(path, "rb");
	gzFile fp=gzdopen(fileno(f), "rb");
	if (fp==Z_NULL) {
		LOG_ERROR("Could not open %s", path.c_str());
		return false;
	}
	char buf[IO_BUF_SIZE];
	repos.clear();
	int i=0;
	while (gzgets(fp, buf, sizeof(buf))!=Z_NULL) {
		std::string tmp=buf;
		std::string url;
		getStrByIdx(tmp,url, ',',1);
		i++;
		if (url.size()>0) { //create new repo from url
			CRepo repotmp=CRepo(url, this);
			repos.push_back(repotmp);
		} else {
			LOG_ERROR("Parse Error %s, Line %d: %s",path.c_str(),i,buf);
			return false;
		}
	}
	gzclose(fp);
	fclose(f);
	LOG_INFO("Found %d repos in %s",repos.size(),path.c_str());
	return true;
}

void CRapidDownloader::updateRepos()
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

