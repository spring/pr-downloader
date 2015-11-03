/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef DOWNLOADENUM_H
#define DOWNLOADENUM_H

#include <string>

namespace DownloadEnum
{
	enum Category {
			CAT_NONE=0,
			CAT_MAPS,
			CAT_GAMES,
			CAT_LUAWIDGETS,
			CAT_AIBOTS,
			CAT_LOBBYCLIENTS,
			CAT_MEDIA,
			CAT_OTHER,
			CAT_REPLAYS,
			CAT_SPRINGINSTALLERS,
			CAT_TOOLS,
			CAT_ENGINE, //is translated to the category which fits best, see below
			CAT_ENGINE_LINUX,
			CAT_ENGINE_LINUX64,
			CAT_ENGINE_WINDOWS,
			CAT_ENGINE_MACOSX,
			CAT_COUNT
	};
	/**
	 *	returns the string name of a category
	 */
	static const std::string getCat(DownloadEnum::Category cat);
	static DownloadEnum::Category getCatFromStr(const std::string& str);

}

#endif
