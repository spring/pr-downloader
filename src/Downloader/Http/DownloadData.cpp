/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "DownloadData.h"

#include <curl/curl.h>

DownloadData::DownloadData()
{
	file=NULL;
	piece=0;
	mirror=0;
	easy_handle=curl_easy_init();
}

DownloadData::~DownloadData()
{
	if (easy_handle!=NULL) {
		curl_easy_cleanup(easy_handle);
		easy_handle=NULL;
	}
}

