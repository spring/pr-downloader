#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include <string>
#include <list>
#include <curl/curl.h>
#include "../../FileSystem.h"
#include "../IDownloader.h"


class CHttpDownloader: public IDownloader{

public:
	CHttpDownloader();
	~CHttpDownloader();

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
	const IDownload* addDownload(const std::string& url, const std::string& filename);
	bool removeDownload(IDownload& download);
	std::list<IDownload>* search(const std::string& name);
	bool start(IDownload* download = NULL);
private:
	CURL *curl;
	unsigned int stats_count;
	unsigned int stats_filepos;
};

#endif
