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
			if (easy_handle!=NULL) {
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
	*	@return true when download_data is correctly set
	*/
	bool setupDownload(CFile& file, download_data* piece, IDownload* download, int mirror);
	bool getRange(std::string& range, int piece, int piecesize, int filesize);
	/**
	* returns piecenum for file, which isn't already downloaded
	* verifies if parts of a file is already downloaded (if checksums are set in download)
	* verified parts are marked with STATE_FINISHED
	* @return number of the piece, -1 if no peaces are avaiable and the whole file needs to be downloaded
	*/
	int verifyAndGetNextPiece(CFile& file, IDownload* download);
	/**
	*	process curl messages
	*		- verify
	*		- starts new pieces, when a piece is finished
	*		- starts redownloads piece, when piece dl failed from some mirror
	*		- keep some stats (mark broken mirrors, downloadspeed)
	*	@returns false, when some fatal error occured -> abort
	*/
	bool processMessages(CURLM* curlm, std::vector <CHttpDownloader::download_data*>& downloads, IDownload* download, CFile& file);
	download_data* getDataByHandle(const std::vector <download_data*>& downloads, const CURL* easy_handle) const;
};

#endif
