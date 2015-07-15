

#include <curl/curl.h>

#include "CurlWrapper.h"
#include "Version.h"

CURL* CurlWrapper::CurlInit()
{
	CURL* ret;
	ret = curl_easy_init();
	curl_easy_setopt(ret, CURLOPT_CONNECTTIMEOUT, 30);

	//if transfer is slower this bytes/s than this for CURLOPT_LOW_SPEED_TIME then its aborted
	curl_easy_setopt(ret, CURLOPT_LOW_SPEED_LIMIT, 10);
	curl_easy_setopt(ret, CURLOPT_LOW_SPEED_TIME, 30);
	curl_easy_setopt(ret, CURLOPT_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
	curl_easy_setopt(ret, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
	curl_easy_setopt(ret, CURLOPT_USERAGENT, getVersion());
	curl_easy_setopt(ret, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(ret, CURLOPT_FOLLOWLOCATION, 1);

	struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Cache-Control: no-cache");
        curl_easy_setopt(ret, CURLOPT_HTTPHEADER, list);
//FIXME: memleak, has to be freed at shutdown
//        curl_slist_free_all(list); /* free the list again */

	return ret;
}
