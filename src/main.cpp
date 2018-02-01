/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Util.h"
#include "Version.h"
#include "Logger.h"
#include "pr-downloader.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <getopt.h>
#include <list>

// TODO: Many of these enums are not implemented.
// e.g. RAPID_VALIDATE_DELETE, RAPID_SEARCH, WIDGET_SEARCH...
enum {
	RAPID_DOWNLOAD = 0,
	RAPID_VALIDATE,
	RAPID_VALIDATE_DELETE,
	RAPID_SEARCH,
	HTTP_SEARCH,
	HTTP_DOWNLOAD,
	WIDGET_SEARCH,
	FILESYSTEM_WRITEPATH,
	FILESYSTEM_DUMPSDP,
	FILESYSTEM_VALIDATESDP,
	DOWNLOAD_MAP,
	DOWNLOAD_GAME,
	DOWNLOAD_ENGINE,
	DISABLE_LOGGING,
	HELP,
	SHOW_VERSION,
	EXTRACT_FILE,
	EXTRACT_DIRECTORY
};

static struct option long_options[] = {
    {"rapid-download", 1, 0, RAPID_DOWNLOAD},
    {"rapid-validate", 0, 0, RAPID_VALIDATE},
    {"delete", 0, 0, RAPID_VALIDATE_DELETE},
    {"dump-sdp", 1, 0, FILESYSTEM_DUMPSDP},
    {"validate-sdp", 1, 0, FILESYSTEM_VALIDATESDP},
    {"http-download", 1, 0, HTTP_DOWNLOAD},
    {"http-search", 1, 0, HTTP_SEARCH},
    {"download-map", 1, 0, DOWNLOAD_MAP},
    {"download-game", 1, 0, DOWNLOAD_GAME},
    {"download-engine", 1, 0, DOWNLOAD_ENGINE},
    {"filesystem-writepath", 1, 0, FILESYSTEM_WRITEPATH},
    {"disable-logging", 0, 0, DISABLE_LOGGING},
    {"help", 0, 0, HELP},
    {"version", 0, 0, SHOW_VERSION},
    {0, 0, 0, 0}};

void show_version()
{
	LOG("pr-downloader %s\n", getVersion());
}

void show_help(const char* cmd)
{
	int i = 0;
	LOG("Usage: %s \n", cmd);
	bool append = false;
	while (long_options[i].name != 0) {
		if (append) {
			LOG("\n");
		}
		append = true;
		LOG("--%s", long_options[i].name);
		if (long_options[i].has_arg != 0)
			LOG(" <name>");
		i++;
	}
	LOG("\n");
	exit(1);
}

void show_results(int count)
{
	for (int i = 0; i < count; i++) {
		downloadInfo dl;
		DownloadGetInfo(i, dl);
		LOG_DEBUG("Download path: %s", dl.filename);
	}
}

bool download(DownloadEnum::Category cat, const char* name)
{
	int count = DownloadSearch(cat, name);
	if (count <= 0) {
		LOG_DEBUG("Couldn't find %s", name);
		return false;
	}
	for (int i = 0; i < count; i++) {
		DownloadAdd(i);
	}
	show_results(count);
	return true;
}

bool search(DownloadEnum::Category cat, const char* name)
{
	int count = DownloadSearch(cat, name);
	if (count <= 0) {
		LOG_ERROR("Couldn't find %s", name);
		return false;
	}
	downloadInfo dl;
	for (int i = 0; i < count; i++) {
		DownloadGetInfo(i, dl);
		LOG("%s\n", dl.filename);
	}
	return true;
}

int main(int argc, char** argv)
{
	show_version();
	if (argc < 2)
		show_help(argv[0]);
	DownloadInit();

	bool res = true;
	bool removeinvalid = false;
	bool fsset = false;

	while (true) {
		const int c = getopt_long(argc, argv, "", long_options, nullptr);
		if (c == -1)
			break;
		switch(c) {
			case RAPID_VALIDATE_DELETE: {
				removeinvalid = true;
				break;
			}
			case FILESYSTEM_WRITEPATH: {
				fsset = true;
				DownloadSetConfig(CONFIG_FILESYSTEM_WRITEPATH, optarg);
				break;
			}
			case DISABLE_LOGGING:
				DownloadDisableLogging(true);
				break;
			default:
				break;
		}
	}
	if (!fsset) {
		DownloadSetConfig(CONFIG_FILESYSTEM_WRITEPATH, "");
	}

	optind = 1; // reset argv scanning
	while (true) {
		const int c = getopt_long(argc, argv, "", long_options, nullptr);
		if (c == -1)
			break;
		switch (c) {
			case RAPID_DOWNLOAD: {
				download(DownloadEnum::CAT_GAME, optarg);
				break;
			}
			case RAPID_VALIDATE: {
				DownloadRapidValidate(removeinvalid);
				break;
			}
			case FILESYSTEM_DUMPSDP: {
				DownloadDumpSDP(optarg);
				break;
			}
			case FILESYSTEM_VALIDATESDP: {
				ValidateSDP(optarg);
				break;
			}
			case DOWNLOAD_MAP: {
				if (!download(DownloadEnum::CAT_MAP, optarg)) {
					LOG_ERROR("No map found for %s", optarg);
					res = false;
				}
				break;
			}
			case DOWNLOAD_GAME: {
				if (!download(DownloadEnum::CAT_GAME, optarg)) {
					LOG_ERROR("No game found for %s", optarg);
					res = false;
				}
				break;
			}
			case SHOW_VERSION:
				show_version();
				break;
			case DOWNLOAD_ENGINE: {
				if (!download(DownloadEnum::CAT_ENGINE, optarg)) {
					LOG_ERROR("No engine version found for %s", optarg);
					res = false;
				}
				break;
			}
			case HTTP_SEARCH: {
				LOG_ERROR("Not implemented");
				break;
			}
			case HELP: {
				show_help(argv[0]);
				break;
			}
		}
	}
	if (optind < argc) {
		while (optind < argc) {
			if (!download(DownloadEnum::CAT_NONE, argv[optind])) {
				LOG_ERROR("No file found for %s", argv[optind]);
				res = false;
			}
			optind++;
		}
	}
	const int dlres = DownloadStart();
	if (dlres > 1) {
		LOG_ERROR("Error occurred while downloading: %d", dlres);
	} else {
		LOG_INFO("Download complete!");
	}
	DownloadShutdown();
	if (res)
		return 0;
	return 1;
}
