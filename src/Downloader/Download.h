/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <stdint.h>
#include "Rapid/Sdp.h"

class DownloadData;
class IHash;
class Mirror;
class CFile;
class IDownloadsObserver;

class IDownload
{
public:
	enum category {
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
		CAT_ENGINE_LINUX,
		CAT_ENGINE_LINUX64,
		CAT_ENGINE_WINDOWS,
		CAT_ENGINE_MACOSX,
		CAT_COUNT
	} cat;

	enum download_type {
		TYP_RAPID,
		TYP_HTTP
	} dltype;

	IDownload(const std::string& filename="",const std::string& orig_name="", category cat=CAT_NONE, download_type typ = TYP_HTTP);
	~IDownload();
	/**
	 *
	 *	add a mirror to the download specified
	 */
	bool addMirror(const std::string& url);
	bool addDepend(const std::string& depend);
	std::string name; //name, in most cases the filename to save to
	std::string origin_name; //name of object. Not the filename

	std::list<std::string> depend; //list of all depends
	bool downloaded; //file was downloaded?
	/**
	 *	returns the string name of a category
	 */
	static const std::string getCat(category cat);
	static IDownload::category getCatFromStr(const std::string& str);
	/**
	 *	returns first url
	 */
	const std::string getUrl();
	Mirror* getMirror(unsigned i) const;
	Mirror* getFastestMirror();
	int getMirrorCount() const;
	/**
	*	size of pieces, last piece size can be different
	*/
	int piecesize;
	enum PIECE_STATE {
		STATE_NONE,        // nothing was done with this piece
		STATE_DOWNLOADING, // piece is currently downloaded
		STATE_FINISHED,    // piece downloaded successfully + verified
	};
	struct piece {
		IHash* sha;
		PIECE_STATE state;
	};
	/**
	 *	sha1 sum of pieces
	 */
	std::vector<struct piece> pieces; //FIXME: make private
	IHash* hash;
	CFile* file;

	/**
	 *	file size
	 */
	int size;


	std::map<CSdp*,uint64_t> rapid_size;
	std::map<CSdp*,uint64_t> map_rapid_progress;

	int progress;
	/**
	 *	state for whole file
	 */
	PIECE_STATE state;
	/**
	 *	returns number of bytes downloaded
	 */
	unsigned int getProgress() const;
	std::string version;

	unsigned int parallel_downloads;
	DownloadData * write_only_from;

private:
	std::vector <Mirror*> mirrors;
	static void initCategories();

};

void SetDownloadsObserver(IDownloadsObserver* ob);

#endif

