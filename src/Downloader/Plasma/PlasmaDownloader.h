#ifndef PLASMA_DOWNLOADER_H
#define PLASMA_DOWNLOADER_H

#include "../IDownloader.h"


class CPlasmaDownloader: public IDownloader {
public:
	CPlasmaDownloader();
	virtual bool search(std::list<IDownload>& result, const std::string& name, IDownload::category cat=IDownload::CAT_NONE);
	virtual bool download(IDownload& download);

private:
	std::string torrentPath;
/**
*
*	parses the bencoded torrent data, strucutre is like this:
*	dict {
*		info => dict {
*			length => int = 21713638
*			name => str = ba750.sdz (len = 9)
*			piece length => int = 262144
*			pieces => str = <sha1 checksums>
*		}
*	}
*
*/
	bool parseTorrent(char*data, int size, IDownload& dl);

};

#endif
