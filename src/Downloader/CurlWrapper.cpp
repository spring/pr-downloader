

#include <curl/curl.h>

#include "CurlWrapper.h"
#include "Util.h"

CURL* CurlWrapper::CurlInit() {
	CURL* ret;
	ret = curl_easy_init();
	curl_easy_setopt(ret, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(ret, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(ret, CURLOPT_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
	curl_easy_setopt(ret, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
	curl_easy_setopt(ret, CURLOPT_USERAGENT, USER_AGENT);
	curl_easy_setopt(ret, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(ret, CURLOPT_FOLLOWLOCATION, 1);
	return ret;
}
