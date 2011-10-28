/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "RapidDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Logger.h"
#include "RepoMaster.h"
#include "Sdp.h"

#include <stdio.h>
#include <string>
#include <string.h>
#include <list>


CRapidDownloader::CRapidDownloader(const std::string& url)
{
	repoMaster=new CRepoMaster(url, this);
	reposLoaded = false;
	sdps.clear();
}

CRapidDownloader::~CRapidDownloader()
{
	sdps.clear();
	delete repoMaster;
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
	repoMaster->updateRepos();
	reposLoaded=true;
	return true;
}


bool CRapidDownloader::download_name(const std::string& longname, int reccounter)
{
	LOG_DEBUG("%s",longname.c_str());
	std::list<CSdp>::iterator it;
	if (reccounter>10)
		return false;
	LOG_INFO("Using rapid");
	for (it=sdps.begin(); it!=sdps.end(); ++it) {
		if (match_download_name((*it).getName(),longname)) {

			LOG_DOWNLOAD((it)->getName().c_str() );
			if (!(*it).download())
				return false;
			if ((*it).getDepends().length()>0) {
				if (!download_name((*it).getDepends(),reccounter+1))
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
			IDownload* dl=new IDownload((*it).getName().c_str());
			dl->addMirror((*it).getShortName().c_str());
			result.push_back(dl);
		}
	}
	return true;
}

bool CRapidDownloader::download(IDownload* download)
{
	LOG_DEBUG("%s",download->name.c_str());
	reloadRepos();
	return download_name(download->name,0);
}
