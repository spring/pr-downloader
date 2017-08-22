#include "pr-downloader.h"
#include "Downloader/IDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Logger.h"
#include "lib/md5/md5.h"
#include "lib/base64/base64.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

static bool fetchDepends = true;

void SetDownloadListener(IDownloaderProcessUpdateListener listener)
{
	IDownloader::setProcessUpdateListener(listener);
}

bool isEngineDownload(DownloadEnum::Category cat)
{
	return (cat == DownloadEnum::CAT_ENGINE_LINUX) ||
	       (cat == DownloadEnum::CAT_ENGINE_LINUX64) ||
	       (cat == DownloadEnum::CAT_ENGINE_MACOSX) ||
	       (cat == DownloadEnum::CAT_ENGINE_WINDOWS) ||
	       (cat == DownloadEnum::CAT_ENGINE_WINDOWS64);
}

DownloadEnum::Category getPlatformEngineCat()
{
#ifdef _WIN64
	return DownloadEnum::CAT_ENGINE_WINDOWS64;
#elif defined WIN32
	return DownloadEnum::CAT_ENGINE_WINDOWS;
#elif defined(__APPLE__)
	return DownloadEnum::CAT_ENGINE_MACOSX;
#elif defined(__x86_64__)
	return DownloadEnum::CAT_ENGINE_LINUX64;
#else
	return DownloadEnum::CAT_ENGINE_LINUX;
#endif
}

bool download_engine(std::list<IDownload*>& dllist)
{
	bool res = true;
	std::list<IDownload*> enginedls;

	for (IDownload* dl : dllist) {
		if (isEngineDownload(dl->cat)) {
			enginedls.push_back(dl);
		}
	}
	if (enginedls.empty())
		return res;

	httpDownload->download(enginedls);
	for (const IDownload* dl : enginedls) {
		if (!fileSystem->extractEngine(dl->name, dl->version)) {
			LOG_ERROR("Failed to extract engine %s", dl->version.c_str());
			res = false;
		}
	}
	return res;
}

// helper function
IDownload* GetIDownloadByID(std::list<IDownload*>& dllist, int id)
{
	std::list<IDownload*>::iterator it;
	int pos = 0;
	for (it = dllist.begin(); it != dllist.end(); ++it) {
		if (pos == id) {
			return *it;
		}
		pos++;
	}
	LOG_ERROR("%s: Couln't find dl %d", __FUNCTION__, id);
	return NULL;
}

std::list<IDownload*> searchres;
std::list<int> downloads;

int DownloadAddByUrl(DownloadEnum::Category cat, const char* filename,
		     const char* url)
{
	IDownload* dl = new IDownload(filename, url, cat);
	dl->addMirror(url);
	searchres.push_back(dl);
	return searchres.size();
}

bool search(DownloadEnum::Category cat, const char* name,
	    std::list<IDownload*>& searchres)
{
	std::string searchname = name;

	if (cat == DownloadEnum::CAT_ENGINE) {
		cat = getPlatformEngineCat();
	}

	switch (cat) {
		case DownloadEnum::CAT_HTTP: // no search possible!
		case DownloadEnum::CAT_SPRINGLOBBY: {
			return false;
		}
		case DownloadEnum::CAT_MAP:
		case DownloadEnum::CAT_ENGINE_LINUX:
		case DownloadEnum::CAT_ENGINE_LINUX64:
		case DownloadEnum::CAT_ENGINE_WINDOWS:
		case DownloadEnum::CAT_ENGINE_WINDOWS64:
		case DownloadEnum::CAT_ENGINE_MACOSX:
			return httpDownload->search(searchres, searchname.c_str(), cat);
		case DownloadEnum::CAT_GAME:
		case DownloadEnum::CAT_COUNT:
		case DownloadEnum::CAT_NONE:
			rapidDownload->search(searchres, searchname.c_str(), cat);
			if (!searchres.empty()) {
				return true;
			}
			httpDownload->search(searchres, searchname.c_str(), cat);
			if (!searchres.empty()) {
				return true;
			}
			return false;
		default:
			LOG_ERROR("%s: type invalid", __FUNCTION__);
			assert(false);
			return false;
	}
	return true;
}

int DownloadSearch(DownloadEnum::Category cat, const char* name)
{
	IDownloader::freeResult(searchres);
	downloads.clear();
	search(cat, name, searchres);
	return searchres.size();
}

bool DownloadGetInfo(int id, downloadInfo& info)
{
	IDownload* dl = GetIDownloadByID(searchres, id);
	if (dl != NULL) {
		strncpy(info.filename, dl->name.c_str(),
			NAME_LEN - 1); // -1 because of 0 termination
		info.cat = dl->cat;
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
	switch (type) {
		case CONFIG_FILESYSTEM_WRITEPATH: {
			std::string path((const char*)value);
			fileSystem->setWritePath(path);
			LOG_INFO("Free disk space: %d MB", CFileSystem::getMBsFree(path));
			return true;
		}
		case CONFIG_FETCH_DEPENDS:
			fetchDepends = (const bool*)value;
			return true;
		case CONFIG_RAPID_FORCEUPDATE:
			rapidDownload->setOption("forceupdate", ""); // FIXME, use value
			return true;
	}
	return false;
}

bool DownloadGetConfig(CONFIG type, const void** value)
{
	switch (type) {
		case CONFIG_FILESYSTEM_WRITEPATH:
			*value = fileSystem->getSpringDir().c_str();
			return true;
		case CONFIG_FETCH_DEPENDS:
			*value = (const bool*)fetchDepends;
			return true;
		case CONFIG_RAPID_FORCEUPDATE:
			// FIXME: implement
			return false;
	}
	return false;
}

bool DownloadAdd(unsigned int id)
{
	if ((id > searchres.size())) {
		LOG_ERROR("%s Invalid id %d", __FUNCTION__, id);
		return false;
	}
	downloads.push_back(id);
	return true;
}

bool dlListContains(const std::list<IDownload*>& dls,
		    const std::string& name)
{
	for (const IDownload* dl : dls) {
		if (dl->name == name) {
			return true;
		}
	}
	return false;
}

bool addDepends(std::list<IDownload*>& dls)
{
	for (const IDownload* dl : dls) {
		for (const std::string& depend : dl->depend) {
			std::list<IDownload*> depends;
			search(DownloadEnum::CAT_COUNT, depend.c_str(), depends);
			LOG_INFO("Adding depend %s", depend.c_str());
			for (IDownload* newdep : depends) {
				if (!dlListContains(dls, newdep->name)) {
					dls.push_back(newdep);
				}
			}
		}
	}
	return true;
}

int DownloadStart()
{
	int res = 0;
	std::list<IDownload*> dls;
	std::list<int>::iterator it;
	const std::string dldir = fileSystem->getSpringDir();
	const unsigned long MBsFree = CFileSystem::getMBsFree(dldir);
	unsigned long dlsize = 0;
	for (it = downloads.begin(); it != downloads.end(); ++it) {
		IDownload* dl = GetIDownloadByID(searchres, *it);
		dlsize += dl->size;
		if (dl == NULL) {
			continue;
		}
		dls.push_back(dl);
	}
	// at least 1024MB free disk space are required (else fragmentation will make file access way to slow!)
	const unsigned long MBsNeeded = (dlsize / (1024 * 1024)) + 1024;

	if (MBsFree < MBsNeeded) {
		LOG_ERROR("Insuffcient free disk space (%d MB) on %s: %d needed", MBsFree, dldir.c_str(), MBsFree);
		return 5;
	}

	if (fetchDepends) {
		addDepends(dls);
	}

	if (dls.empty()) {
		LOG_DEBUG("Nothing to do, did you forget to call DownloadAdd()?");
		res = 1;
		return res;
	}
	if (!rapidDownload->download(dls))
		res = 2;
	if (!httpDownload->download(dls, 1))
		res = 3;
	if (!download_engine(dls))
		res = 4;

	IDownloader::freeResult(searchres);
	dls.clear();
	return res;
}

bool DownloadRapidValidate(bool deletebroken)
{
	std::string path = fileSystem->getSpringDir();
	path += PATH_DELIMITER;
	path += "pool";
	return fileSystem->validatePool(path, deletebroken);
}

bool DownloadDumpSDP(const char* path)
{
	return fileSystem->dumpSDP(path);
}

bool ValidateSDP(const char* path)
{
	return fileSystem->validateSDP(path);
}

void DownloadDisableLogging(bool disableLogging)
{
	LOG_DISABLE(disableLogging);
}

// TODO: Add support for other hash types (SHA1, CRC32, ..?)
char* CalcHash(const char* str, int size, int type)
{
	const unsigned char* hash;
	MD5_CTX ctx;
	if (type == 0) {
		MD5Init(&ctx);
		MD5Update(&ctx, (unsigned char*)str, size);
		MD5Final(&ctx);
		hash = ctx.digest;
	} else {
		return NULL;
	}

	std::string encoded = base64_encode(hash, 16);
	char* ret = (char*)malloc((encoded.size() + 1) * sizeof(char));
	strncpy(ret, encoded.c_str(), encoded.size());
	ret[encoded.size()] = '\0';
	return ret;
}

void SetAbortDownloads(bool value)
{
	IDownloader::SetAbortDownloads(value);
}
