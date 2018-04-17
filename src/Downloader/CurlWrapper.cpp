/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <curl/curl.h>

#include "CurlWrapper.h"
#include "Version.h"
#include "FileSystem/HashSHA1.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "IDownloader.h"
#include "Logger.h"

constexpr const char* cacertpem = "https://curl.haxx.se/ca/cacert-2018-03-07.pem";
constexpr const char* cacertfile = "cacert.pem";
constexpr const char* cacertsha1 = "f8d2bb6dde84f58b2c8caf584eaf0c040e7afc97";


bool CurlWrapper::VerifyFile(const std::string& path)
{
	if (!fileSystem->fileExists(path))
		return false;
	HashSHA1 hash;
	hash.Init();

	FILE* f = fileSystem->propen(path, "rb");
	if (f == nullptr)
		return false;
	char buf[1024];
	int items;
	do {
		items = fread(buf, 1, sizeof(buf), f);
		hash.Update(buf, items);
	} while(items > 0);
	hash.Final();
	if (cacertsha1 == hash.toString())
		return true;

	LOG_INFO("%s compare failed: %s != %s", path.c_str(), cacertsha1, hash.toString().c_str());
	return false;
}

bool CurlWrapper::ValidateCaFile()
{
	cafile = fileSystem->getSpringDir() + PATH_DELIMITER + cacertfile;
	if (VerifyFile(cafile))
		return true;

	IDownload tmpdl(cafile);
	tmpdl.addMirror(cacertpem);
	tmpdl.validateTLS = false;
	LOG_INFO("Downloading %s", cacertpem);
        if(!httpDownload->download(&tmpdl))
		return false;
	if (VerifyFile(cafile)) {
		return true;
	}
	LOG_ERROR("Verification of downloaded %s failed, please delete for automatic redownload", cafile.c_str());
	return false;
}

CurlWrapper::CurlWrapper()
{
	static bool fetched = false;
	if (!fetched) {
		fetched = true;
		ValidateCaFile();
	}

	handle = curl_easy_init();
	if (fileSystem->fileExists(cafile)) {
		LOG_INFO("Using certstore %s", cafile.c_str());
		curl_easy_setopt(handle, CURLOPT_CAINFO, cafile.c_str());
	}

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

	list = nullptr;
	list = curl_slist_append(list, "Cache-Control: no-cache");
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, list);
}

CurlWrapper::~CurlWrapper()
{
	curl_slist_free_all(list); /* free the list again */
	curl_easy_cleanup(handle);
	handle = nullptr;
	list = nullptr;
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
