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
	virtual bool download(IDownload& download);
	virtual std::list<IDownload>* search(const std::string& name, IDownload::category=IDownload::CAT_NONE);
private:
	/**
		returns the bytes of the torrent already downloaded
	*/
	int getProcess(const libtorrent::torrent_handle& torrentHandle);
};
#endif
