#ifndef RAPID_DOWNLOADER_H
#define RAPID_DOWNLOADER_H


#define MAX_DATA 5
#include <string>
#include <list>
#include <stdio.h>
//FIXME: read from config / use this as default
#define REPO_MASTER "http://repos.caspring.org/repos.gz"

class CSdp;
class CHttpDownload;
class CRepoMaster;
class CFileSystem;

class CRapidDownloader{
	static CRapidDownloader* singleton;

public:
	static inline CRapidDownloader* GetInstance() {
		return singleton;
	}
	static void Initialize();
	static void Shutdown();
	//lists a tag, for exampe ba:stable
	bool download_tag(const std::string& modname);
	//lists all tags on all servers
	void list_tag();
	//remove a dsp from the list of remote dsps
	void addRemoteDsp(CSdp& dsp);
	CRapidDownloader();
private:
	std::list<CSdp*> sdps;
	bool reloadRepos();
	bool reposLoaded;
};

#define rapidDownloader CRapidDownloader::GetInstance()

#endif
