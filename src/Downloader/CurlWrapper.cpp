/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <curl/curl.h>

#include "CurlWrapper.h"
#include "Version.h"

CurlWrapper::CurlWrapper()
{
	handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 30);

	// if transfer is slower this bytes/s than this for CURLOPT_LOW_SPEED_TIME
	// then its aborted
	curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, 10);
	curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, 30);
	curl_easy_setopt(handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
	curl_easy_setopt(handle, CURLOPT_REDIR_PROTOCOLS,
			 CURLPROTO_HTTP | CURLPROTO_HTTPS);
	curl_easy_setopt(handle, CURLOPT_USERAGENT, getVersion());
	curl_easy_setopt(handle, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);

	list = NULL;
	list = curl_slist_append(list, "Cache-Control: no-cache");
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, list);
}

CurlWrapper::~CurlWrapper()
{
	curl_easy_cleanup(handle);
	handle = NULL;
	curl_slist_free_all(list); /* free the list again */
}

std::string CurlWrapper::escapeUrl(const std::string& url)
{
	std::string res;
	for (unsigned int i = 0; i < url.size();
	     i++) { // FIXME: incomplete, needs to support all unicode chars
		if (url[i] == ' ')
			res.append("%20");
		else
			res.append(1, url[i]);
	}
	return res;
}
