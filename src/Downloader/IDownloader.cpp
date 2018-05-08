/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "IDownloader.h"
#include "Download.h"
#include "Http/HttpDownloader.h"
#include "Rapid/RapidDownloader.h"
#include "Util.h"
#include "Logger.h"
#include "Mirror.h"
#include <curl/curl.h>
#include <cassert>

class IDownloader;

IDownloader* IDownloader::httpdl = NULL;
IDownloader* IDownloader::rapiddl = NULL;
IDownloaderProcessUpdateListener IDownloader::listener = nullptr;


static void DumpTLSInfo()
{
#if CURL_AT_LEAST_VERSION(7,56,0)
	const curl_ssl_backend **list;
	int i;
	const int res = curl_global_sslset((curl_sslbackend)-1, nullptr, &list);
	if (res == CURLSSLSET_UNKNOWN_BACKEND) {
			for(i = 0; list[i]; i++) {
			LOG_INFO("SSL backend #%d: '%s' (ID: %d)\n", i, list[i]->name, list[i]->id);
		}
	} else {
		LOG_WARN("Cannot enumerate ssl backends");
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

void IDownloader::Initialize()
{

	DumpVersion();
	DumpTLSInfo();
	curl_global_init(CURL_GLOBAL_ALL);
}

void IDownloader::Shutdown()
{
	delete (httpdl);
	httpdl = NULL;
	delete (rapiddl);
	rapiddl = NULL;
	curl_global_cleanup();
}
static bool abortDownloads = false;
void IDownloader::SetAbortDownloads(bool value)
{
	abortDownloads = value;
}

bool IDownloader::AbortDownloads()
{
	return abortDownloads;
}

IDownloader* IDownloader::GetHttpInstance()
{
	if (httpdl == NULL)
		httpdl = new CHttpDownloader();
	return httpdl;
}
IDownloader* IDownloader::GetRapidInstance()
{
	if (rapiddl == NULL)
		rapiddl = new CRapidDownloader();
	return rapiddl;
}

bool IDownloader::download(std::list<IDownload*>& download, int max_parallel)
{
	if (download.empty()) {
		LOG_ERROR("download list empty");
		return false;
	}
	bool res = true;
	for (IDownload* dl : download) {
		if (dl->state == IDownload::STATE_FINISHED) // don't download twice
			continue;

		if (!this->download(dl, max_parallel))
			res = false;
	}
	return res;
}

bool IDownloader::download(IDownload* dl, int max_parallel)
{
	std::list<IDownload*> dls;
	dls.push_back(dl);
	return download(dls, max_parallel);
}

void IDownloader::freeResult(std::list<IDownload*>& list)
{
	std::list<IDownload*>::iterator it;
	for (it = list.begin(); it != list.end(); ++it) {
		delete *it;
	}
	list.clear();
}

bool IDownloader::setOption(const std::string& key, const std::string& value)
{
	LOG_ERROR("Invalid option: %s=%s", key.c_str(), value.c_str());
	return false;
}

void IDownloader::setProcessUpdateListener(IDownloaderProcessUpdateListener l)
{
	IDownloader::listener = l;
}
