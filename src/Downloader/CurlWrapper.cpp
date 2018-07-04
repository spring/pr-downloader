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


#ifndef CURL_VERSION_BITS
#define CURL_VERSION_BITS(x,y,z) ((x)<<16|(y)<<8|z)
#endif

#ifndef CURL_AT_LEAST_VERSION
#define CURL_AT_LEAST_VERSION(x,y,z) \
  (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(x, y, z))
#endif

static std::string GetCAFilePath()
{
	return fileSystem->getSpringDir() + PATH_DELIMITER + cacertfile;
}

static void DumpTLSInfo()
{
#if CURL_AT_LEAST_VERSION(7,60,0)
	const curl_ssl_backend **list;
	curl_global_sslset((curl_sslbackend)-1, nullptr, &list);
	for(int i = 0; list[i]; i++) {
		LOG_INFO("SSL backend #%d: '%s' (ID: %d)", i, list[i]->name, list[i]->id);
	}
#endif
}



static void DumpVersion()
{
	const curl_version_info_data* ver = curl_version_info(CURLVERSION_NOW);
	if ((ver != nullptr) && (ver->age > 0)) {
		LOG_INFO("libcurl %s %s", ver->version, ver->ssl_version);
	}
}

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

bool CurlWrapper::ValidateCaFile(const std::string& cafile)
{
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

	handle = curl_easy_init();

	const std::string cafile = GetCAFilePath();
	if (!fileSystem->fileExists(cafile)) {
		LOG_WARN("Certstore doesn't exist: %s", cafile.c_str());
	}
	const int res = curl_easy_setopt(handle, CURLOPT_CAINFO, cafile.c_str());
	if (res != CURLE_OK) {
		LOG_ERROR("Error setting CURLOPT_CAINFO: %d", res);
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

void CurlWrapper::InitCurl()
{
	DumpVersion();
	DumpTLSInfo();
	curl_global_init(CURL_GLOBAL_ALL);
	ValidateCaFile(GetCAFilePath());
}

void CurlWrapper::KillCurl()
{
	curl_global_cleanup();
}

