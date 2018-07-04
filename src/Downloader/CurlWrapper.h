/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <curl/curl.h>
#include <string>

class CurlWrapper
{
public:
	CurlWrapper();
	~CurlWrapper();
	CURL* GetHandle() const
	{
		return handle;
	}
	static std::string escapeUrl(const std::string& url);
	static void InitCurl();
	static void KillCurl();
private:
	static bool VerifyFile(const std::string& path);
	static bool ValidateCaFile(const std::string& cafile);

	CURL* handle;
	struct curl_slist* list;
};
