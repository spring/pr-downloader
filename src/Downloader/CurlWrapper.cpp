/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <curl/curl.h>
#include <cstring>

#include "CurlWrapper.h"
#include "Version.h"
#include "FileSystem/HashSHA1.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "IDownloader.h"
#include "Logger.h"

constexpr const char* cacertpem = "https://springrts.com/dl/cacert.pem";
constexpr const char* cacertfile = "cacert.pem";
//constexpr const char* cacertsha1 = "fe1e06f7048b78dbe7015c1c043de957251181db";
constexpr const char* capath = "/etc/ssl/certs";


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

static curl_sslbackend backend = CURLSSLBACKEND_NONE;

static void GetTLSBackend()
{
#if CURL_AT_LEAST_VERSION(7,60,0)
	const curl_ssl_backend **list;
	curl_global_sslset((curl_sslbackend)-1, nullptr, &list);
	for(int i = 0; list[i]; i++) {
		LOG_INFO("SSL backend #%d: '%s' (ID: %d)", i, list[i]->name, list[i]->id);
		if (backend == CURLSSLBACKEND_NONE) { // use first as res
			backend = list[i]->id;
		} else {
			LOG_WARN("Multiple SSL backends, this will very likely fail!");
		}
	}
#else
#warning Compiling without full curl support: please upgrade to libcurl >= 7.60
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
	if (fileSystem->getFileSize(path) <= 0)
		return false;
	if (fileSystem->isOlder(path, 15552000)) {
		LOG_INFO("%s is older than half a year, redownloading" ,path.c_str());
		return false;
	}
/*
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
*/
	return true;
}

bool CurlWrapper::ValidateCaFile(const std::string& cafile)
{
	if (VerifyFile(cafile))
		return true;

	IDownload tmpdl(cafile);
	tmpdl.addMirror(cacertpem);
	tmpdl.validateTLS = false;
	LOG_INFO("Downloading %s", cacertpem);
        if(!httpDownload->download(&tmpdl)) {
		LOG_ERROR("Download of %s to %s failed, please manually download!", cacertpem, GetCAFilePath().c_str());
		return false;
	}
	return true;
}

static void SetCAOptions(CURL* handle)
{
	if (fileSystem->directoryExists(capath)) {
		LOG_DEBUG("Using capath: %s", capath);
		const int res = curl_easy_setopt(handle, CURLOPT_CAPATH, capath);
		if (res != CURLE_OK) {
			LOG_WARN("Error setting CURLOPT_CAPATH: %d", res);
		}
		return;
	}

	const std::string cafile = GetCAFilePath();
	if (!fileSystem->fileExists(cafile)) {
		LOG_WARN("Certstore doesn't exist: %s", cafile.c_str());
	}
	const int res = curl_easy_setopt(handle, CURLOPT_CAINFO, cafile.c_str());
	if (res != CURLE_OK) {
		LOG_WARN("Error setting CURLOPT_CAINFO: %d", res);
	}
}

CurlWrapper::CurlWrapper()
{
	handle = curl_easy_init();
	errbuf = (char*)malloc(sizeof(char) * CURL_ERROR_SIZE);
	errbuf[0] = 0;
	curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, errbuf);

	if (backend != CURLSSLBACKEND_SCHANNEL) {
		SetCAOptions(handle);
	}

	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 30);

	// if transfer is slower this bytes/s than this for CURLOPT_LOW_SPEED_TIME
	// then its aborted
	curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, 10);
	curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, 30);
	curl_easy_setopt(handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
	curl_easy_setopt(handle, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
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
	free(errbuf);
	errbuf = nullptr;
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
	GetTLSBackend();
	curl_global_init(CURL_GLOBAL_ALL);
	if (backend != CURLSSLBACKEND_SCHANNEL) {
		ValidateCaFile(GetCAFilePath());
	}
}

void CurlWrapper::KillCurl()
{
	curl_global_cleanup();
}

std::string CurlWrapper::GetError() const
{
	if (errbuf == nullptr)
		return "";
	return std::string(errbuf, strlen(errbuf));
}

