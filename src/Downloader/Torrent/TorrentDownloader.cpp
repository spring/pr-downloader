#include <string>
#include "TorrentDownloader.h"

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"


bool CTorrentDownloader::download(const std::string& torrentfile, const std::string& filename){
	libtorrent::session s;
	s.listen_on(std::make_pair(6881, 6889));
	libtorrent::add_torrent_params p;
	p.save_path = "./";
    p.ti = new libtorrent::torrent_info(torrentfile.c_str());
    libtorrent::torrent_handle tor=s.add_torrent(p);

//    tor.add_url_seed();

    while(true){
    	const libtorrent::session_status& sessinfo=s.status();
    	sleep(1);
    	printf("%d\n",sessinfo.total_download);

    }
}


void CTorrentDownloader::start(IDownload* download){
}

const IDownload* CTorrentDownloader::addDownload(const std::string& url, const std::string& filename){
}

bool CTorrentDownloader::removeDownload(IDownload& download){
}

const IDownload* CTorrentDownloader::search(const std::string& name){
}
