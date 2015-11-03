/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <map>
#include <string>

#include "DownloadEnum.h"
#include "Logger.h"

static std::map<int, std::string> categories;

static void initCategories()
{
	if (!categories.empty()) { //not initialized yet
		return;
	}
	categories[DownloadEnum::CAT_MAP] ="map";
	categories[DownloadEnum::CAT_GAME] ="game";
	categories[DownloadEnum::CAT_SPRINGLOBBY] = "springlobby";
	categories[DownloadEnum::CAT_ENGINE] = "engine";
	categories[DownloadEnum::CAT_ENGINE_LINUX] = "engine_linux";
	categories[DownloadEnum::CAT_ENGINE_LINUX64] = "engine_linux64";
	categories[DownloadEnum::CAT_ENGINE_WINDOWS] =  "engine_windows";
	categories[DownloadEnum::CAT_ENGINE_MACOSX] = "engine_macosx";
	categories[DownloadEnum::CAT_HTTP] = "http";
	categories[DownloadEnum::CAT_COUNT] = "count";
}

const std::string DownloadEnum::getCat(DownloadEnum::Category cat)
{
	initCategories();
	return categories[cat];
}

DownloadEnum::Category DownloadEnum::getCatFromStr(const std::string& str)
{
	initCategories();
	for(const auto &myPair: categories) {
		if (myPair.second == str) {
			return (DownloadEnum::Category)myPair.first;
		}
	}
	LOG_ERROR("Unknown category: %s", str.c_str());
	return DownloadEnum::CAT_NONE;
}
