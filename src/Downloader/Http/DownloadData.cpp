/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "DownloadData.h"

#include <curl/curl.h>
#include "Downloader/CurlWrapper.h"

DownloadData::DownloadData()
{
	start_piece=0;
	mirror=NULL;
	download=NULL;
	easy_handle=CurlWrapper::CurlInit();
	got_ranges = false;
}

DownloadData::~DownloadData()
{
	if (easy_handle!=NULL) {
		curl_easy_cleanup(easy_handle);
		easy_handle=NULL;
	}
}

