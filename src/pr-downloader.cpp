#include "pr-downloader.h"
#include "Downloader/IDownloader.h"
#include "FileSystem/FileSystem.h"

#include <string.h>
#include <stdlib.h>

std::list<IDownload*> downloads;
int downloadSearch(category cat, char* name)
{
	switch(cat) {
	case CAT_MAP:
		httpDownload->search(downloads, name);
		break;
	case CAT_GAME:
		httpDownload->search(downloads, name);
	}
	return downloads.size();
}

bool downloadGetSearchInfo(downloadInfo* info, int id)
{
	std::list<IDownload*>::iterator it;
	int pos=0;
	for(it=downloads.begin(); it!=downloads.end(); ++it) {
		if (pos==id) {
//		info->speed =(*it)->dlspeed;
			strncpy(info->filename, (*it)->name.c_str(), NAME_LEN);
			return true;
		}
		pos++;
	}
	return false;
}

bool download(int id)
{
	std::list<IDownload*>::iterator it;
	int pos=0;
	for(it=downloads.begin(); it!=downloads.end(); ++it) {
		if (pos==id) {
			httpDownload->download(*it); //FIXME: could be deleted, when search is called while downloading
			return true;
		}
		pos++;
	}
	return false;
}

void downloadInit()
{
	CFileSystem::Initialize();
	IDownloader::Initialize();
}

void downloadShutdown()
{
	std::list<IDownload*>::iterator it;
	for(it=downloads.begin(); it!=downloads.end(); ++it) {
		free(*it);
	}
	downloads.clear();
	IDownloader::Shutdown();
	CFileSystem::Shutdown();
}

bool downloadSetStr(char* name, char* value)
{
	if (strcmp(name, "filesystem-writepath")) {
		fileSystem->Initialize(value);
		return true;
	}
	return false;
}
