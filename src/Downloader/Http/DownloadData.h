/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _DOWNLOAD_DATA_H
#define _DOWNLOAD_DATA_H

#include <vector>
class CFile;
class Mirror;
class IDownload;
class CurlWrapper;

class DownloadData
{
public:
	DownloadData();
	~DownloadData();
	int start_piece;
	std::vector<unsigned int> pieces;
	CurlWrapper* curlw; // curl_easy_handle
	Mirror* mirror;     // mirror used
	IDownload* download;
	bool got_ranges; // true if headers received from server are fine
};

#endif
