#ifndef _DOWNLOAD_DATA_H
#define _DOWNLOAD_DATA_H

#include <string>
class CFile;
typedef void CURL;

class DownloadData
{
public:
	DownloadData();
	~DownloadData();
	CFile* file;
	int piece;
	CURL* easy_handle; //curl_easy_handle
	int mirror; //number of mirror used
	std::string url; //copy of the download url
};

#endif
