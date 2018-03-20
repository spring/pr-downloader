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

private:
	bool VerifyFile(const std::string& path);
	bool ValidateCaFile();

	CURL* handle;
	struct curl_slist* list;
	std::string cafile;
};
