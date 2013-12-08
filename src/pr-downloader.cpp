#include "pr-downloader.h"
#include "Downloader/IDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Logger.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>


static bool fetchDepends = true;

bool download_engine(std::list<IDownload*>& dllist)
{
	httpDownload->download(dllist);
	bool res = true;
	std::list<IDownload*>::iterator it;
	for (it = dllist.begin(); it!=dllist.end(); ++it) {
		if (!fileSystem->extractEngine((*it)->name, (*it)->version))
			res = false;
	}
	return res;
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

IDownload::category getCat(category cat) //FIXME: unify enums, see IDownload::category
{
	switch (cat) {
	case CAT_MAP:
		return IDownload::CAT_MAPS;
	case CAT_GAME:
		return IDownload::CAT_GAMES;
	case CAT_ENGINE:
#ifdef WIN32
		return IDownload::CAT_ENGINE_WINDOWS;
#elif defined(__APPLE__)
		return IDownload::CAT_ENGINE_MACOSX;
#elif defined(__x86_64__)
		return IDownload::CAT_ENGINE_LINUX64;
#else
		return IDownload::CAT_ENGINE_LINUX;
#endif

	case CAT_ANY:
		return IDownload::CAT_NONE;
	default:
		LOG_ERROR("Invalid category: %d", cat);
	}
	return IDownload::CAT_NONE;
}

bool isEngineDownload(IDownload::category cat)
{
	return (cat == IDownload::CAT_ENGINE_LINUX)   ||
	       (cat == IDownload::CAT_ENGINE_LINUX64) ||
	       (cat == IDownload::CAT_ENGINE_MACOSX)  ||
	       (cat == IDownload::CAT_ENGINE_WINDOWS);
}


std::list<IDownload*> searchres;
downloadtype typ;

bool search(downloadtype type, category cat, const char* name, std::list<IDownload*>& searchres)
{
	IDownload::category icat = getCat(cat);
	if (isEngineDownload(icat)) { //engine downloads only work with http
		type = DL_ENGINE;
		LOG_ERROR("engine dl");
	}
	typ = type;
	std::string searchname = name;

	switch(type) {
	case DL_RAPID:
		return rapidDownload->search(searchres, searchname.c_str(), icat);
		break;
	case DL_HTTP:
	case DL_ENGINE:
		return httpDownload->search(searchres, searchname.c_str(), icat);
		break;
	case DL_PLASMA:
		return plasmaDownload->search(searchres, searchname.c_str(), icat);
		break;
	case DL_ANY:
		rapidDownload->search(searchres, searchname.c_str(), icat);
		if (!searchres.empty()) {
			typ = DL_RAPID;
			break;
		}
		typ = DL_HTTP;
		httpDownload->search(searchres, searchname.c_str(), icat);
		if (!searchres.empty()) {
			break;
		}
		//last try, use plasma
		return plasmaDownload->search(searchres, searchname.c_str(), icat);
		break;
	default:
		LOG_ERROR("%s: type invalid", __FUNCTION__);
		return false;
	}
	return true;
}

int DownloadSearch(downloadtype type, category cat, const char* name)
{
	IDownloader::freeResult(searchres);
	search(type, cat, name, searchres);
	return searchres.size();
}

bool DownloadGetSearchInfo(int id, downloadInfo& info)
{
	IDownload* dl = GetIDownloadByID(searchres, id);
	if (dl!=NULL) {
		strncpy(info.filename, dl->name.c_str(), NAME_LEN-1); // -1 because of 0 termination
		return true;
	}
	return false;
}

void DownloadInit()
{
	IDownloader::Initialize();
}

void DownloadShutdown()
{
	IDownloader::freeResult(searchres);
	IDownloader::Shutdown();
	CFileSystem::Shutdown();
}

bool DownloadSetConfig(CONFIG type, const void* value)
{
	switch(type) {
	case CONFIG_FILESYSTEM_WRITEPATH:
		fileSystem->setWritePath((const char*)value);
		return true;
	case CONFIG_FETCH_DEPENDS:
		fetchDepends = (const bool*) value;
		return true;
	}
	return false;
}

bool DownloadGetConfig(CONFIG type, const void** value)
{
	switch(type) {
	case CONFIG_FILESYSTEM_WRITEPATH:
		*value = fileSystem->getSpringDir().c_str();
		return true;
	case CONFIG_FETCH_DEPENDS:
		*value = (const bool*)fetchDepends;
		return true;
	}
	return false;
}

std::list<int> downloads;

bool DownloadAdd(unsigned int id)
{
	if ((id>searchres.size())) {
		LOG_ERROR("%s Invalid id %d", __FUNCTION__, id);
		return false;
	}
	downloads.push_back(id);
	return true;
}

bool addDepends(std::list<IDownload*>& dls)
{
	std::list<IDownload*>::iterator it;
	for (it = dls.begin(); it != dls.end(); ++it) {
		if ((*it)->depend.empty()) {
			continue;
		}
		std::list<std::string>::iterator stit;
		for (stit = (*it)->depend.begin(); stit != (*it)->depend.end(); ++stit) {
			std::list<IDownload*> depends;
			const std::string& depend = (*stit);
			search(DL_ANY, CAT_ANY, depend.c_str(), depends);
			LOG_INFO("Adding depend %s", depend.c_str());
			dls.merge(depends);
		}
	}
	return true;
}

bool DownloadStart()
{
	bool res = true;
	std::list<IDownload*> dls;
	std::list<int>::iterator it;
	for (it = downloads.begin(); it != downloads.end(); ++it) {
		IDownload* dl = GetIDownloadByID(searchres, *it);
		if (dl == NULL) {
			continue;
		}
		dls.push_back(dl);
	}
	if (fetchDepends) {
		addDepends(dls);
	}

	if (dls.empty()) {
		LOG_DEBUG("Nothing to do, did you forget to call DownloadAdd()?");
		return res;
	}

	switch (typ) {
	case DL_RAPID:
	case DL_HTTP:
		if(!rapidDownload->download(dls))
			res = false;
		if (!httpDownload->download(dls,1))
			res = false;
		break;
	case DL_ENGINE:
		if (!download_engine(dls))
			res = false;
		break;
	default:
		LOG_ERROR("%s():%d  Invalid type specified: %d %d", __FUNCTION__, __LINE__, typ, downloads.size());
		res = false;
	}

	IDownloader::freeResult(searchres);
	dls.clear();
	return res;
}

bool DownloadRapidValidate()
{
	std::string path = fileSystem->getSpringDir();
	path += PATH_DELIMITER;
	path += "pool";
	return fileSystem->validatePool(path);
}

bool DownloadDumpSDP(const char* path)
{
	return fileSystem->dumpSDP(path);
}

void DownloadDisableLogging(bool disableLogging)
{
	LOG_DISABLE(disableLogging);
}

