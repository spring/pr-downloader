#ifndef TORRENT_DOWNLOADER_H
#define TORRENT_DOWNLOADER_H

#include <string>
#include "../IDownloader.h"

#define TORRENT_DISABLE_GEO_IP 1

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>

class CTorrentDownloader: public IDownloader{
public:
	bool download(const std::string& torrentfile, const std::string& filename="");
	bool start(IDownload* download = NULL);
	const IDownload* addDownload(const std::string& url, const std::string& filename="");
	bool removeDownload(IDownload& download);
	std::list<IDownload>* search(const std::string& name, IDownload::category=IDownload::CAT_NONE);
private:
	/**
		returns the bytes of the torrent already downloaded
	*/
	int getProcess(const libtorrent::torrent_handle& torrentHandle);
};
#endif
