/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Downloader/IDownloader.h"
#include "Downloader/Mirror.h"
#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Logger.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <list>


enum {
	RAPID_DOWNLOAD=0,
	RAPID_VALIDATE,
	RAPID_SEARCH,
	HTTP_SEARCH,
	HTTP_DOWNLOAD,
	PLASMA_DOWNLOAD,
	PLASMA_SEARCH,
	WIDGET_SEARCH,
	FILESYSTEM_WRITEPATH,
	FILESYSTEM_DUMPSDP,
	DOWNLOAD_MAP,
	DOWNLOAD_GAME,
	HELP,
	SHOW_VERSION
};

static struct option long_options[] = {
	{"rapid-download"          , 1, 0, RAPID_DOWNLOAD},
	{"rapid-validate"          , 0, 0, RAPID_VALIDATE},
	{"rapid-search"            , 1, 0, RAPID_SEARCH},
	{"plasma-download"         , 1, 0, PLASMA_DOWNLOAD},
	{"plasma-search"           , 1, 0, PLASMA_SEARCH},
	{"http-download"           , 1, 0, HTTP_DOWNLOAD},
	{"http-search"             , 1, 0, HTTP_SEARCH},
	{"download-map"            , 1, 0, DOWNLOAD_MAP},
	{"download-game"           , 1, 0, DOWNLOAD_GAME},
	{"widget-search"           , 1, 0, WIDGET_SEARCH},
	{"filesystem-writepath"    , 1, 0, FILESYSTEM_WRITEPATH},
	{"filesystem-dumpsdp"      , 1, 0, FILESYSTEM_DUMPSDP},
	{"help"                    , 0, 0, HELP},
	{"version"                 , 0, 0, SHOW_VERSION},
	{0                         , 0, 0, 0}
};

void show_version()
{
	LOG("pr-downloader " VERSION "\n");
}

void show_help(const char* cmd)
{
	show_version();
	int i=0;
	LOG("Usage: %s \n", cmd);
	bool append=false;
	while (long_options[i].name!=0) {
		if (append) {
			LOG("\n");
		}
		append=true;
		LOG("--%s",long_options[i].name);
		if (long_options[i].has_arg!=0)
			LOG(" <name>");
		i++;
	}
	LOG("\n");
	exit(1);
}

bool download(const std::string& name, IDownload::category cat)
{
	std::list<IDownload*> res;
	//only games can be (currently) downloaded by rapid
	if (cat==IDownload::CAT_MODS) {
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

void show_results(std::list<IDownload*>& list)
{
	std::list<IDownload*>::iterator it;
	for (it=list.begin(); it!=list.end(); ++it) {
		const int size = (*it)->size;
		const char* name = (*it)->name.c_str();
		LOG_INFO("Filename: %s Size: %d",name, size);
		int count=(*it)->getMirrorCount();
		for(int i=0; i<count; i++) {
			LOG_INFO("Download url: %s",(*it)->getMirror(i)->url.c_str());
		}
	}
}

int main(int argc, char **argv)
{
	if (argc<2)
		show_help(argv[0]);

	CFileSystem::Initialize();
	IDownloader::Initialize();

	while (true) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "",long_options, &option_index);
		if (c == -1)
			break;
		std::list<IDownload*> list;
		list.clear();
		switch (c) {
		case RAPID_DOWNLOAD: {
			rapidDownload->search(list, optarg);
			if (list.empty()) {
				LOG_ERROR("Coulnd't find %s",optarg);
			} else if (!rapidDownload->download(list)) {
				LOG_ERROR("Error downloading %s",optarg);
			}
			IDownloader::freeResult(list);
			break;
		}
		case RAPID_VALIDATE: {
			int res=fileSystem->validatePool(fileSystem->getSpringDir() + PATH_DELIMITER + "pool");
			LOG_INFO("Validated %d files",res);
			break;
		}
		case RAPID_SEARCH: {
			std::string tmp=optarg;
			rapidDownload->search(list, tmp);
			show_results(list);
			IDownloader::freeResult(list);
			break;
		}
		case PLASMA_SEARCH: {
			std::string tmp=optarg;
			plasmaDownload->search(list, tmp);
			show_results(list);
			IDownloader::freeResult(list);
			break;
		}
		case PLASMA_DOWNLOAD: {
			plasmaDownload->search(list, optarg);
			if (!list.empty())
				plasmaDownload->download(list);
			IDownloader::freeResult(list);
			break;
		}
		case WIDGET_SEARCH: {
			std::string tmp=optarg;
			widgetDownload->search(list, tmp);
			IDownloader::freeResult(list);
			break;
		}
		case FILESYSTEM_WRITEPATH: {
			std::string tmp=optarg;
			CFileSystem::Initialize(optarg);
			break;
		}
		case FILESYSTEM_DUMPSDP: {
			fileSystem->dumpSDP(optarg);
			break;
		}
		case HTTP_SEARCH: {
			httpDownload->search(list, optarg);
			show_results(list);
			IDownloader::freeResult(list);
			break;
		}
		case HTTP_DOWNLOAD: {
			httpDownload->search(list, optarg);
			httpDownload->download(list);
			IDownloader::freeResult(list);
			break;
		}
		case DOWNLOAD_MAP: {
			if (!download(optarg, IDownload::CAT_MAPS)) {
				LOG_ERROR("No map found for %s",optarg);
			}
			break;
		}
		case DOWNLOAD_GAME: {
			if (!download(optarg, IDownload::CAT_MODS)) {
				LOG_ERROR("No game found for %s",optarg);
			}
			break;
		}
		case SHOW_VERSION:
			show_version();
			break;
		case HELP:
		default: {
			show_help(argv[0]);
			break;
		}
		}
	}

	if (optind < argc) {
		while (optind < argc) {
			std::string tmp = argv[optind];
			if (!download(tmp, IDownload::CAT_MODS)) {
				LOG_ERROR("No file found for %s",optarg);
			}
			optind++;
		}
	}

	IDownloader::Shutdown();
	CFileSystem::Shutdown();

	return 0;
}
