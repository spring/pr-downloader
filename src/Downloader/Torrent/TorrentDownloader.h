#ifndef TORRENT_DOWNLOADER_H
#define TORRENT_DOWNLOADER_H

#include <string>
#include "Downloader/IDownloader.h"


class CTorrentDownloader: public IDownloader{
public:
	bool download(const std::string& torrentfile, const std::string& filename="");
	void start(IDownload* download = NULL);
	const IDownload* addDownload(const std::string& url, const std::string& filename="");
	bool removeDownload(IDownload& download);
	const std::list<IDownload>* search(const std::string& name);
};
#endif
