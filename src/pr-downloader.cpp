#include "pr-downloader.h"
#include "Downloader/IDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Logger.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>


bool download(const std::string& name, IDownload::category cat)
{
	std::list<IDownload*> res;
	//only games can be (currently) downloaded by rapid
	if ((cat==IDownload::CAT_GAMES) || (cat == IDownload::CAT_NONE)) {
		rapidDownload->search(res, name, cat);
		if ((!res.empty()) && (rapidDownload->download(res)))
			return true;
	}
	httpDownload->search(res, name, cat);
	if ((!res.empty())) {
		std::list<IDownload*>::iterator dlit;
		for(dlit=res.begin(); dlit!=res.end(); ++dlit) { //download depends, too. we handle this here, because then we can use all dl-systems
			if (!(*dlit)->depend.empty()) {
				std::list<std::string>::iterator it;
				for(it=(*dlit)->depend.begin(); it!=(*dlit)->depend.end(); ++it) {
					const std::string& depend = (*it);
					LOG_INFO("found depends: %s", depend.c_str());
					if (!download(depend, cat)) {
						LOG_ERROR("downloading the depend %s for %s failed", depend.c_str(), name.c_str());
						return false;
					}
				}
			}
		}
		return httpDownload->download(res);
	}
	plasmaDownload->search(res, name, cat);
	if ((!res.empty()))
		return plasmaDownload->download(res);
	return false;
}

bool download_engine(std::string& version)
{
	IDownload::category cat;
#ifdef WIN32
	cat=IDownload::CAT_ENGINE_WINDOWS;
#elif __APPLE__
	cat=IDownload::CAT_ENGINE_MACOSX;
#else
	cat=IDownload::CAT_ENGINE_LINUX;
#endif

	std::list<IDownload*> res;
	httpDownload->search(res, "spring " + version, cat);
	if (res.empty()) {
		return false;
	}
	std::list<IDownload*>::iterator it;
	it = res.begin();
	if (!httpDownload->download(*it)) {
		return false;
	}
	const std::string output = fileSystem->getSpringDir() + PATH_DELIMITER + "engine" + PATH_DELIMITER + version;
	return fileSystem->extract((*it)->name, output);
}

//helper function
IDownload* GetIDownloadByID(std::list<IDownload*>& dllist, int id)
{
	std::list<IDownload*>::iterator it;
	int pos=0;
	for(it=dllist.begin(); it!=dllist.end(); ++it) {
		if (pos==id) {
			return *it;
		}
		pos++;
	}
	LOG_ERROR("%s: Couln't find dl %d", __FUNCTION__, id);
	return NULL;
}

std::list<IDownload*> searchres;
downloadtype typ;
int DownloadSearch(downloadtype type, category cat, const char* name)
{
	IDownloader::freeResult(searchres);
	typ = type;

	switch(type) {
	case DL_RAPID:
		rapidDownload->search(searchres, name);
		break;
	case DL_HTTP:
		httpDownload->search(searchres, name);
		break;
	case DL_PLASMA:
		plasmaDownload->search(searchres, name);
		break;
	case DL_ANY:
		rapidDownload->search(searchres, name);
		if (!searchres.empty()) {
			typ = DL_RAPID;
			break;
		}
		typ = DL_HTTP;
		httpDownload->search(searchres, name);
		if (!searchres.empty()) {
			break;
		}
		//last try, use plasma
		plasmaDownload->search(searchres, name);
		break;
	default:
		LOG_ERROR("%s: type invalid", __FUNCTION__);
	}
	return searchres.size();
}

bool DownloadGetSearchInfo(int id, downloadInfo& info)
{
	IDownload* dl = GetIDownloadByID(searchres, id);
	if (dl!=NULL) {
		strncpy(info.filename, dl->name.c_str(), NAME_LEN);
		return true;
	}
	return false;
}

void DownloadInit()
{
	CFileSystem::Initialize();
	IDownloader::Initialize();
}

void DownloadShutdown()
{
	/*	if (!downloads.empty()) {
			LOG_ERROR("downloads !empty");
		}
		IDownloader::freeResult(downloads);*/
	IDownloader::freeResult(searchres);
	IDownloader::Shutdown();
	CFileSystem::Shutdown();
}

bool DownloadSetConfig(CONFIG type, const void* value)
{
	switch(type) {
	case CONFIG_FILESYSTEM_WRITEPATH:
		fileSystem->Initialize((const char*)value);
		return true;
	}
	return false;
}

std::list<int> downloads;

bool DownloadAdd(int id)
{
	downloads.push_back(id);
	return true;
}

bool DownloadStart()
{
	std::list<IDownload*> dls;
	std::list<int>::iterator it;
	for (it = downloads.begin(); it != downloads.end(); ++it) {
		IDownload* dl = GetIDownloadByID(searchres, *it);
		if (dl == NULL)
			continue;
		dls.push_back(dl);
	}
	switch (typ) {
	case DL_RAPID:
		return rapidDownload->download(dls);
	case DL_HTTP:
		return httpDownload->download(dls);
	default:
		LOG_ERROR("%s():%d  Invalid type specified: %d %d", __FUNCTION__, __LINE__, typ, downloads.size());
		return false;
	}
	return false;
}
