#include <string>
#include "TorrentDownloader.h"

#include "../../FileSystem.h"
#include "../../Util.h"

bool CTorrentDownloader::download(const std::string& torrentfile, const std::string& filename){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return true;
}

int CTorrentDownloader::getProcess(const libtorrent::torrent_handle& torrentHandle){
	std::vector<libtorrent::size_type> progress;
	torrentHandle.file_progress(progress);
	int size=progress.size();
	int sum=0;
	for (int i=0; i<size;i++){
		sum=sum+progress.at(i);
	}
	return sum;
}

bool CTorrentDownloader::start(IDownload* download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	if (download==NULL)
		return false;
	libtorrent::session torrentSession;

//	s=new libtorrent::session();

    libtorrent::session_settings setting;
    setting.tracker_completion_timeout=1;
    setting.stop_tracker_timeout=1;
    setting.peer_timeout=1;
    setting.urlseed_timeout=1;

	torrentSession.set_settings(setting);

	torrentSession.listen_on(std::make_pair(6881, 6889));

	libtorrent::add_torrent_params addTorrentParams;
	addTorrentParams.save_path = download->name; //name contains the path, because torrents already include the filenames
    addTorrentParams.ti = new libtorrent::torrent_info(download->url.c_str());
	for (int i=0; i<addTorrentParams.ti->num_files(); i++){
		printf("File %d in torrent: %s\n",i, addTorrentParams.ti->file_at(i).path.filename().c_str());
	}

    printf("Downloading torrent to %s\n", download->name.c_str());
    libtorrent::torrent_handle torrentHandle=torrentSession.add_torrent(addTorrentParams);

    std::list<std::string>::iterator it;
	for(it=download->mirror.begin(); it!=download->mirror.end(); it++){
		printf("Adding webseed to torrent %s\n",(*it).c_str());
		urlEncode(*it);
		torrentHandle.add_url_seed(*it);
	}
	libtorrent::torrent_info torrentInfo = torrentHandle.get_torrent_info();

	if (addTorrentParams.ti->num_files()==1){ //try http-download because only 1 mirror exists
		it=download->mirror.begin();
		httpDownload->addDownload(*it,download->name + addTorrentParams.ti->file_at(0).path.filename());
		return httpDownload->start();
	}

    while( (!torrentHandle.is_finished()) && (!torrentHandle.is_seed()) && (torrentHandle.is_valid())){
//		const libtorrent::session_status& sessionStatus=torrentSession.status();
		printf("\r%d/%ld",getProcess(torrentHandle), torrentInfo.total_size());
		fflush(stdout);
		libtorrent::time_duration time(1000000); //1 sec
		const libtorrent::alert* a = torrentSession.wait_for_alert(time);
		if (a!=NULL){
			printf("peer error: %d %d\n",a->category(),libtorrent::alert::peer_notification | libtorrent::alert::error_notification );

			if (( a->category() &  libtorrent::alert::peer_notification) &&
				(  a->category() & libtorrent::alert::error_notification)){
				printf("wrong peer found!\n");
				break; //better fail than hang
			}
			printf("%s\n",a->message().c_str());
			torrentSession.pop_alert();
		}
    }

    printf("download finished, shuting down torrent...\n");
    torrentSession.pause();
	printf("shut down!\n");
	return true;
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
