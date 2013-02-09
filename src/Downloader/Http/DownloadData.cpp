/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "DownloadData.h"

#include <curl/curl.h>

DownloadData::DownloadData()
{
	piece=0;
	mirror=NULL;
	download=NULL;
	easy_handle=curl_easy_init();
	curl_easy_setopt(easy_handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
	curl_easy_setopt(easy_handle, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
	headersok = false;
}

DownloadData::~DownloadData()
{
	if (easy_handle!=NULL) {
		curl_easy_cleanup(easy_handle);
		easy_handle=NULL;
	}
}

