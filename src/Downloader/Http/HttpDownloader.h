/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include "../IDownloader.h"

#include <curl/curl.h>
#include <string>
#include <list>

class HashMD5;
class HashSHA1;
class CFile;
class FileData;
class DownloadData;

class CHttpDownloader: public IDownloader
{

public:
	CHttpDownloader();
	~CHttpDownloader();

	/**
		downloads a file from Url to filename
	*/
	//FIXME: this hides diff signature from IDownloader
	bool downloadbyurl(const std::string& Url, const std::string& filename, int pos=1);
	void setCount(unsigned int count);
	void setStatsPos(unsigned int pos);
	unsigned int getStatsPos();
	unsigned int getCount();
	const std::string& getCacheFile(const std::string &url);
	virtual bool search(std::list<IDownload*>& result, const std::string& name, DownloadEnum::Category=DownloadEnum::CAT_NONE);
	virtual bool download(std::list< IDownload* >& download, int max_parallel=10);
	void showProcess(IDownload* download, bool forceOutput);
	static bool DownloadUrl(const std::string& url, std::string& res);
	static bool ParseResult(const std::string& name, const std::string& json, std::list<IDownload*>& res);
private:
	bool parallelDownload(IDownload& download);
	std::string escapeUrl(const std::string& url);
	/**
	* show progress bar
	*/


	/**
	*	gets next piece that can be downloaded, mark it as downloading
	*	@return true when DownloadData is correctly set
	*/
	bool setupDownload(DownloadData* piece);
	bool getRange(std::string& range, int start_piece,int num_pieces, int piecesize);
	/**
	* returns piecenum for file, which isn't already downloaded
	* verifies if parts of a file is already downloaded (if checksums are set in download)
	* verified parts are marked with STATE_FINISHED
	* @return number of the piece, -1 if no peaces are avaiable and the whole file needs to be downloaded
	*/
	std::vector< unsigned int > verifyAndGetNextPieces(CFile& file, IDownload* download);
	/**
	*	process curl messages
	*		- verify
	*		- starts new pieces, when a piece is finished
	*		- starts redownloads piece, when piece dl failed from some mirror
	*		- keep some stats (mark broken mirrors, downloadspeed)
	*	@returns false, when some fatal error occured -> abort
	*/
	bool processMessages(CURLM* curlm, std::vector <DownloadData*>& downloads);
	DownloadData* getDataByHandle(const std::vector <DownloadData*>& downloads, const CURL* easy_handle) const;
};

#endif
