/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <curl/curl.h>

class CurlWrapper
{
public:
	CurlWrapper();
	~CurlWrapper();
	CURL* const GetHandle() const {
		return handle;
	}
private:
	CURL* handle;
	struct curl_slist *list;
};
