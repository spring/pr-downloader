#include <string>
#include "TorrentDownloader.h"

#include "../../FileSystem.h"
#include "../../Util.h"

bool CTorrentDownloader::download(IDownload& download){
	DEBUG_LINE("%s",download.name.c_str());

	libtorrent::add_torrent_params addTorrentParams;
	addTorrentParams.save_path = download.name; //name contains the path, because torrents already include the filenames
	addTorrentParams.ti = new libtorrent::torrent_info(download.url.c_str());
	for (int i=0; i<addTorrentParams.ti->num_files(); i++){
		DEBUG_LINE("File %d in torrent: %s",i, addTorrentParams.ti->file_at(i).path.filename().c_str());
	}

	bool res=true;
	std::list<std::string>::iterator it;
	it=download.mirror.begin();
	srand ( time(NULL) );
	int pos=rand() % download.mirror.size();
	for (int i=0; i<pos; i++) //use random mirror
		it++;
	IDownload dl(*it,download.name + addTorrentParams.ti->file_at(0).path.filename());
	printf("%d\n",pos);
	return httpDownload->download(dl);

/*
//FIXME: make torrent work (+ quick shutdown)

	libtorrent::session* torrentSession;

	libtorrent::session_settings setting;
	setting.tracker_completion_timeout=1;
	setting.stop_tracker_timeout=1;
	setting.peer_timeout=1;
	setting.urlseed_timeout=1;
	setting.user_agent="pr-downloader";

	torrentSession = new libtorrent::session();
	torrentSession->set_settings(setting);

	printf("Downloading torrent to %s\n", download.name.c_str());
	libtorrent::torrent_handle torrentHandle=torrentSession->add_torrent(addTorrentParams);

	for(it=download.mirror.begin(); it!=download.mirror.end(); ++it){
		printf("Adding webseed to torrent %s\n",(*it).c_str());
		urlEncode(*it);
		torrentHandle.add_url_seed(*it);
	}

	torrentSession->listen_on(std::make_pair(6881, 6889));
	while( (!torrentHandle.is_finished()) && (!torrentHandle.is_seed()) && (torrentHandle.is_valid())){
		libtorrent::session_status sessionStatus = torrentSession->status();
		printf("\r%ld/%ld               ",sessionStatus.total_download, torrentHandle.get_torrent_info().total_size());
		fflush(stdout);
		libtorrent::time_duration time(1000000); // 1 sec
		const libtorrent::alert* a = torrentSession->wait_for_alert(time);
		if (a!=NULL){
			if (a->category() & libtorrent::alert::error_notification)
				printf(" error");
			if (a->category() & libtorrent::alert::peer_notification)
				printf(" peer");
			if (a->category() &  libtorrent::alert::port_mapping_notification)
				printf(" port_mapping");
			if (a->category() &  libtorrent::alert::storage_notification)
				printf(" storage");
			if (a->category() &  libtorrent::alert::tracker_notification)
				printf(" tracker");
			if (a->category() &  libtorrent::alert::debug_notification)
				printf(" debug");
			if (a->category() &  libtorrent::alert::status_notification)
				printf(" status");
			if (a->category() &  libtorrent::alert::progress_notification)
				printf(" progress");
			if (a->category() &  libtorrent::alert::ip_block_notification)
				printf(" ip_block");
			if (a->category() &  libtorrent::alert::performance_warning)
				printf(" performance");
			if (a->category() &  libtorrent::alert::all_categories)
				printf(" all");
			if (( a->category() &  libtorrent::alert::peer_notification) &&
				(  a->category() & libtorrent::alert::error_notification)){
				printf(" error downloading torrent(%d): %s\n",a->category(),a->message().c_str());
				res=false;
				break; 
				
			}

			printf(" : %s\n",a->message().c_str());
			torrentSession->pop_alert();
		}
	}

	printf("download finished, shuting down torrent...\n");
	torrentSession->pause();
	delete torrentSession;
	printf("shut down!\n");
*/
	return res;
}

std::list<IDownload>* CTorrentDownloader::search(const std::string& name, IDownload::category cat){
	DEBUG_LINE("%s",name.c_str());
	return NULL;
}
