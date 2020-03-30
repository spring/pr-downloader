/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <stdint.h>
#include "Rapid/Sdp.h"
#include "DownloadEnum.h"

class DownloadData;
class IHash;
class Mirror;
class CFile;

class IDownload
{
public:
	DownloadEnum::Category cat;

	enum download_type { TYP_RAPID,
			     TYP_HTTP } dltype;

	IDownload(const std::string& filename = "", const std::string& orig_name = "",
		  DownloadEnum::Category cat = DownloadEnum::CAT_NONE,
		  download_type typ = TYP_HTTP);
	~IDownload();
	/**
   *
   *	add a mirror to the download specified
   */
	bool addMirror(const std::string& url);
	bool addDepend(const std::string& depend);
	std::string name;	// name, in most cases the filename to save to
	std::string origin_name; // name of object. Not the filename

	std::list<std::string> depend; // list of all depends
				       /**
                                  *	returns first url
                                  */
	std::string getUrl() const;
	Mirror* getMirror(unsigned i) const;
	Mirror* getFastestMirror();
	int getMirrorCount() const;
	/**
  *	size of pieces, last piece size can be different
  */
	int piecesize = 0;
	enum PIECE_STATE {
		STATE_NONE,	// nothing was done with this piece
		STATE_DOWNLOADING, // piece is currently downloaded
		STATE_FINISHED,    // piece downloaded successfully + verified
	};
	struct piece
	{
		IHash* sha;
		PIECE_STATE state;
	};
	/**
   *	sha1 sum of pieces
   */
	std::vector<struct piece> pieces; // FIXME: make private
	IHash* hash = nullptr;
	CFile* file = nullptr;

	/**
   *	file size
   */
	int size = -1;

	std::map<CSdp*, uint64_t> rapid_size;
	std::map<CSdp*, uint64_t> map_rapid_progress;

	int progress = 0;
	/**
   *	state for whole file
   */
	PIECE_STATE state = IDownload::STATE_NONE;
	/**
   *	returns number of bytes downloaded
   */
	unsigned int getProgress() const;
	std::string version;

	unsigned int parallel_downloads = 0;
	DownloadData* write_only_from = nullptr;

	bool validateTLS = true;
private:
	std::vector<Mirror*> mirrors;
	static void initCategories();
};

#endif
