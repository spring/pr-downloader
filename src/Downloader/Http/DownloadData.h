/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _DOWNLOAD_DATA_H
#define _DOWNLOAD_DATA_H

#include <string>
#include <vector>
class CFile;
class Mirror;
class IDownload;
typedef void CURL;

class DownloadData
{
public:
	DownloadData();
	~DownloadData();
	int start_piece;
	std::vector<unsigned int> pieces;
	CURL* easy_handle; //curl_easy_handle
	Mirror* mirror; //mirror used
	IDownload *download;
	bool got_ranges; //true if headers received from server are fine
};

#endif
