#include <string>
#include "TorrentDownloader.h"

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"


bool CTorrentDownloader::download(const std::string& torrentfile, const std::string& filename){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return true;
}

void CTorrentDownloader::start(IDownload* download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	if (download==NULL)
		return;
	libtorrent::session s;

    libtorrent::session_settings setting;
    setting.tracker_completion_timeout=1;
    setting.stop_tracker_timeout=1;
    setting.peer_timeout=1;
    setting.urlseed_timeout=1;

	s.set_settings(setting);

	s.listen_on(std::make_pair(6881, 6889));

	libtorrent::add_torrent_params p;
	p.save_path = download->name; //name contains the path, because torrents already include the filenames
    p.ti = new libtorrent::torrent_info(download->url.c_str());
    printf("Downloading torrent to %s\n", download->name.c_str());
    libtorrent::torrent_handle tor=s.add_torrent(p);

    std::list<std::string>::iterator it;
	for(it=download->mirror.begin(); it!=download->mirror.end(); it++){
		printf("Adding webseed to torrent %s\n",(*it).c_str());
    	tor.add_url_seed(*it);
	}

    while( (!tor.is_seed()) && (tor.is_valid())){
    	const libtorrent::session_status& sessinfo=s.status();
    	sleep(1);
    	printf("%ld\n",sessinfo.total_download);

    }


	s.abort();

    printf("download finished\n");
}

const IDownload* CTorrentDownloader::addDownload(const std::string& url, const std::string& filename){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return NULL;
}

bool CTorrentDownloader::removeDownload(IDownload& download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return true;
}

std::list<IDownload>* CTorrentDownloader::search(const std::string& name){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return NULL;
}
