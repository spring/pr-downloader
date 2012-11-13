/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

//#include "Downloader/IDownloader.h"
//#include "Downloader/Mirror.h"
//#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Logger.h"
#include "pr-downloader.h"

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
	DOWNLOAD_ENGINE,
	HELP,
	SHOW_VERSION,
	EXTRACT_FILE,
	EXTRACT_DIRECTORY
};

static struct option long_options[] = {
	{"rapid-download"          , 1, 0, RAPID_DOWNLOAD},
	{"rapid-validate"          , 0, 0, RAPID_VALIDATE},
	{"dump-sdp"                , 1, 0, FILESYSTEM_DUMPSDP},
	{"plasma-download"         , 1, 0, PLASMA_DOWNLOAD},
	{"http-download"           , 1, 0, HTTP_DOWNLOAD},
	{"http-search"             , 1, 0, HTTP_SEARCH},
	{"download-map"            , 1, 0, DOWNLOAD_MAP},
	{"download-game"           , 1, 0, DOWNLOAD_GAME},
	{"download-engine"         , 1, 0, DOWNLOAD_ENGINE},
	{"filesystem-writepath"    , 1, 0, FILESYSTEM_WRITEPATH},
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

void show_results(int count)
{
	for(int i=0; i<count; i++) {
		downloadInfo dl;
		DownloadGetSearchInfo(i, dl);
		LOG_INFO("Download path: %s", dl.filename);
	}
}

bool download(category cat, const char* name)
{
	int count = DownloadSearch(DL_ANY, cat, name);
	if (count<=0) {
		LOG_ERROR("Couldn't find %s",name);
		return false;
	}
	for (int i=0; i<count; i++) {
		DownloadAdd(i);
	}
	show_results(count);
	return true;
}

bool search(category cat, const char* name)
{
	int count = DownloadSearch(DL_ANY, cat, name);
	if (count<=0) {
		LOG_ERROR("Couldn't find %s",name);
		return false;
	}
	downloadInfo dl;
	for (int i=0; i<count; i++) {
		DownloadGetSearchInfo(i, dl);
		LOG("%s", dl.filename);
	}
	return true;
}

int main(int argc, char **argv)
{
	show_version();
	if (argc<2)
		show_help(argv[0]);
	DownloadInit();

	bool res=true;
	while (true) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case RAPID_DOWNLOAD: {
			download(CAT_GAME, optarg);
			break;
		}
		case RAPID_VALIDATE: {
			DownloadRapidValidate();
			break;
		}
		case FILESYSTEM_DUMPSDP: {
			DownloadDumpSDP(optarg);
			break;
		}
		case FILESYSTEM_WRITEPATH: {
			DownloadSetConfig(CONFIG_FILESYSTEM_WRITEPATH, optarg);
			break;
		}
		case HTTP_SEARCH: {
			search(CAT_ANY, optarg);
			break;
		}
		case HTTP_DOWNLOAD: {
			if (!download(CAT_ANY, optarg))
				res = false;
			break;
		}
		case DOWNLOAD_MAP: {
			if (!download(CAT_MAP, optarg)) {
				LOG_ERROR("No map found for %s",optarg);
				res=false;
			}
			break;
		}
		case DOWNLOAD_GAME: {
			if (!download(CAT_GAME, optarg)) {
				LOG_ERROR("No game found for %s",optarg);
				res=false;
			}
			break;
		}
		case SHOW_VERSION:
			show_version();
			break;
		case DOWNLOAD_ENGINE: {
			char buf[1024];
			snprintf(buf, sizeof(buf), "spring %s", optarg);
			if (!download(CAT_ENGINE, buf)) {
				LOG_ERROR("No engine version found for %s",optarg);
				res=false;
			}
			break;
		}
		case HELP:
		default: {
			show_help(argv[0]);
			break;
		}
		}
	}
	if (optind < argc) {
		while (optind < argc) {
			if (!download(CAT_ANY, argv[optind])) {
				LOG_ERROR("No file found for %s",argv[optind]);
				res=false;
			}
			optind++;
		}
	}
	DownloadStart();
	DownloadShutdown();
	if (res)
		return 0;
	return 1;
}
