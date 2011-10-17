#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include <string>
#include <list>
#include <curl/curl.h>
#include "FileSystem/FileSystem.h"
#include "Downloader/IDownloader.h"

#define MAX_PARALLEL_DOWNLOADS 10

class HashMD5;
class HashSHA1;
class CFile;

class CHttpDownloader: public IDownloader
{

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
	void downloadStream(const std::string& url,std::list<FileData*>& files);
	virtual bool search(std::list<IDownload*>& result, const std::string& name, IDownload::category=IDownload::CAT_NONE);
	virtual bool download(IDownload* download);

	class download_data
	{
	public:
		download_data() {
			file=NULL;
			piece=0;
			mirror=0;
			easy_handle=curl_easy_init();
		}
		~download_data() {
			if (easy_handle!=NULL){
				curl_easy_cleanup(easy_handle);
				easy_handle=NULL;
			}
		}
		CFile* file;
		int piece;
		CURL* easy_handle; //curl_easy_handle
		int mirror; //number of mirror used
	};

private:
	CURL* curl;
	bool parallelDownload(IDownload& download);
	unsigned int stats_count;
	unsigned int stats_filepos;
	unsigned long lastprogress; //last time progress bar was shown
	std::list<IDownload>* realSearch(const std::string& name, IDownload::category cat);
	std::string escapeUrl(const std::string& url);
	/**
	* show progress bar
	*/
	void showProcess(IDownload* download, CFile& file);

	/**
	*	gets next piece that can be downloaded, mark it as downloading
	*/
	bool getPiece(CFile& file, download_data* piece, IDownload* download, int mirror);
	bool getRange(std::string& range, int piece, int piecesize, int filesize);
};

#endif
