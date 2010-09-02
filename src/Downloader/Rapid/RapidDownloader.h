#ifndef RAPID_DOWNLOADER_H
#define RAPID_DOWNLOADER_H


#include <string>
#include <list>
#include <stdio.h>
//FIXME: read from config / use this as default
#define REPO_MASTER "http://repos.caspring.org/repos.gz"
#include "Downloader/IDownloader.h"

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
	virtual void addRemoteDsp(CSdp& dsp);
	virtual const IDownload* addDownload(const std::string& url, const std::string& filename);
	virtual bool removeDownload(IDownload& download);
	virtual const std::list<IDownload>* search(const std::string& name);
	virtual void start(IDownload* download);

private:
	std::list<CSdp*> sdps;
	bool reloadRepos();
	bool reposLoaded;
	CRepoMaster* repoMaster;
};

#endif
