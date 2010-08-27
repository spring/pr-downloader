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

	bool download_revision(const std::string& mirror,const std::string& package, const std::string& springwritedir);
	bool download_tag(const char* modname);
	void list_tag();
	void addRemoteDsp(CSdp& dsp);
	void removeRemoteDsp(CSdp& sdp);
private:
	std::list<CSdp*> sdps;
};

#define rapidDownloader CRapidDownloader::GetInstance()

#endif
