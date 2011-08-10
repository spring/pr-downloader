#ifndef RAPID_DOWNLOADER_H
#define RAPID_DOWNLOADER_H


#include <string>
#include <list>
#include <stdio.h>
//FIXME: read from config / use this as default
#define REPO_MASTER "http://repos.caspring.org/repos.gz"
#define REPO_MASTER_RECHECK_TIME 3600 //how long to cache the repo-master file in secs without rechecking
#define REPO_RECHECK_TIME 600

#include "../IDownloader.h"

class CSdp;
class CHttpDownload;
class CRepoMaster;
class CFileSystem;

class CRapidDownloader: public IDownloader{
public:
	CRapidDownloader();
	~CRapidDownloader();

	//lists a tag, for exampe ba:stable
	bool download_tag(const std::string& modname);
	//lists all tags on all servers
	void list_tag();
	//remove a dsp from the list of remote dsps
	void addRemoteDsp(CSdp* dsp);
	virtual bool search(std::list<IDownload>& result, const std::string& name, IDownload::category=IDownload::CAT_NONE);
	virtual bool download(IDownload& download);

private:
	bool download_name(const std::string& longname, int reccounter=0);
	bool reloadRepos();
	bool reposLoaded;
	CRepoMaster* repoMaster;
	std::list<CSdp*> sdps;
};

#endif
