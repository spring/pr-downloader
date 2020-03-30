/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <map>
#include <string>

#include "DownloadEnum.h"
#include "Logger.h"

static std::map<DownloadEnum::Category, std::string> categories;

static void initCategories()
{
	if (!categories.empty()) {
		return;
	}

	categories[DownloadEnum::CAT_MAP] = "map";
	categories[DownloadEnum::CAT_GAME] = "game";
	categories[DownloadEnum::CAT_SPRINGLOBBY] = "springlobby";
	categories[DownloadEnum::CAT_ENGINE] = "engine";
	categories[DownloadEnum::CAT_ENGINE_LINUX] = "engine_linux";
	categories[DownloadEnum::CAT_ENGINE_LINUX64] = "engine_linux64";
	categories[DownloadEnum::CAT_ENGINE_WINDOWS] = "engine_windows";
	categories[DownloadEnum::CAT_ENGINE_WINDOWS64] = "engine_windows64";
	categories[DownloadEnum::CAT_ENGINE_MACOSX] = "engine_macosx";
	categories[DownloadEnum::CAT_HTTP] = "http";
	categories[DownloadEnum::CAT_COUNT] = "count";
}

namespace DownloadEnum {

std::string getCat(Category cat)
{
	initCategories();
	return categories[cat];
}

Category getCatFromStr(const std::string& str)
{
	initCategories();
	for (const auto& myPair : categories) {
		if (myPair.second == str) {
			return myPair.first;
		}
	}
	LOG_ERROR("Unknown category: %s", str.c_str());
	return CAT_NONE;
}

}