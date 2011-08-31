#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <string>
#include <list>

class IDownload{
public:
	enum category{
		CAT_NONE=0,
		CAT_MAPS,
		CAT_MODS,
		CAT_LUAWIDGETS,
		CAT_AIBOTS,
		CAT_LOBBYCLIENTS,
		CAT_MEDIA,
		CAT_OTHER,
		CAT_REPLAYS,
		CAT_SPRINGINSTALLERS,
		CAT_TOOLS
	}cat;

	IDownload(const std::string& filename="", category cat=CAT_NONE);
	/**
	 *
	 *	add a mirror to the download specified
	 */
	bool addMirror(const std::string& url);
	bool addDepend(const std::string& depend);
	std::list<std::string> mirror;
	std::string name; //name, in most cases the filename to save to
	std::list<std::string> depend; //list of all depends
	bool downloaded; //file was downloaded?
	/**
	 *	returns the string name of a category
	 */
	const std::string& getCat(category cat);
	const std::string& getUrl();

};

#endif
