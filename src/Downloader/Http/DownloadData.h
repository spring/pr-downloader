/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _DOWNLOAD_DATA_H
#define _DOWNLOAD_DATA_H

#include <memory>
#include <vector>

class Mirror;
class IDownload;
class CurlWrapper;

class DownloadData
{
public:
	DownloadData();

	int start_piece = 0;
	std::vector<unsigned int> pieces;
	std::unique_ptr<CurlWrapper> curlw; // curl_easy_handle
	Mirror* mirror = nullptr;     // mirror used
	IDownload* download;
	bool got_ranges = false; // true if headers received from server are fine
};

#endif
