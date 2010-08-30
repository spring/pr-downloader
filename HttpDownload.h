#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include <string>
#include <list>
#include <curl/curl.h>
#include "FileSystem.h"




class CHttpDownload{
	static CHttpDownload* singleton;

public:
	static inline CHttpDownload* GetInstance() {
		return singleton;
	}
	static void Initialize();
	static void Shutdown();
	CHttpDownload();
	~CHttpDownload();

	/**
		downloads a file from Url to filename
	*/
	bool download(const std::string& Url, const std::string& filename, int pos=1);
	void setCount(unsigned int count);
	void setStatsPos(unsigned int pos);
	unsigned int getStatsPos();
	unsigned int getCount();
	const std::string& getCacheFile(const std::string &url);
	void downloadStream(const std::string url,std::list<CFileSystem::FileData*>& files);
private:
	CURL *curl;
	unsigned int stats_count;
	unsigned int stats_filepos;
};


#define httpDownload CHttpDownload::GetInstance()

#endif
