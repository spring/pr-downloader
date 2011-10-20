/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _DOWNLOAD_DATA_H
#define _DOWNLOAD_DATA_H

#include <string>
class CFile;
class Mirror;
typedef void CURL;

class DownloadData
{
public:
	DownloadData();
	~DownloadData();
	CFile* file;
	int piece;
	CURL* easy_handle; //curl_easy_handle
	Mirror* mirror; //mirror used
};

#endif
