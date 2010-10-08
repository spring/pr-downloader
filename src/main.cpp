#define VERSION 0.1
#define USER_AGENT "unitsync-dev" + VERSION
#define SPRING_DIR "/home/matze/.spring"
#define TMP_FILE "/tmp/repos.gz"
#define TMP_FILE2 "/tmp/version.gz"


#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "md5.h"
#include "Util.h"
#include <string>
#include "FileSystem.h"
#include <unistd.h>
#include <getopt.h>
#include <Downloader/IDownloader.h>
#include <stdio.h>
#include <list>

enum{
	RAPID_DOWNLOAD=0,
	RAPID_VALIDATE,
	RAPID_LIST,
	HTTP_SEARCH,
	HTTP_DOWNLOAD,
	PLASMA_DOWNLOAD,
	PLASMA_SEARCH,
	TORRENT_DOWNLOAD,
	WIDGET_SEARCH,
	FILESYSTEM_WRITEPATH,
	DOWNLOAD_MAP,
	DOWNLOAD_MOD,
	HELP
};

static struct option long_options[] = {
	{"rapid-download"          , 1, 0, RAPID_DOWNLOAD},
	{"rapid-validate"          , 0, 0, RAPID_VALIDATE},
	{"rapid-list"              , 0, 0, RAPID_LIST},
	{"plasma-download"         , 1, 0, PLASMA_DOWNLOAD},
	{"plasma-search"           , 1, 0, PLASMA_SEARCH},
	{"http-download"           , 1, 0, HTTP_DOWNLOAD},
	{"http-search"             , 1, 0, HTTP_SEARCH},
	{"torrent-download"        , 1, 0, TORRENT_DOWNLOAD},
	{"download-map"            , 1, 0, DOWNLOAD_MAP},
	{"download-mod"            , 1, 0, DOWNLOAD_MOD},
	{"widget-search"           , 1, 0, WIDGET_SEARCH},
	{"filesystem-writepath"    , 0, 0, FILESYSTEM_WRITEPATH},
	{"help"                    , 0, 0, HELP},
	{0                         , 0, 0, 0}
};

void show_help(const char* cmd){
	int i=0;
	printf("Usage: %s ", cmd);
	bool append=false;
	while(long_options[i].name!=0){
		if (append){
			printf("\n");
		}
		append=true;
		printf("--%s",long_options[i].name);
		if (long_options[i].has_arg!=0)
			printf(" <name>");
		i++;
	}
	printf("\n");
	exit(1);
}

bool download(const std::string& name, IDownload::category cat){
	std::list<IDownload>* res=rapidDownload->search(optarg, cat);
	if ((res!=NULL) && (!res->empty())) && (rapidDownload->download(*res)))
		return true;
	res=plasmaDownload->search(optarg, cat);
	if ((res!=NULL) && (!res->empty())) && plasmaDownload->download(*res))
		return true;
	res=httpDownload->search(optarg, cat);
	if ((res!=NULL) && (!res->empty()))
		return httpDownload->download(*res);
	return false;
}

int main(int argc, char **argv){
	if (argc<2)
		show_help(argv[0]);

	CFileSystem::Initialize();
	IDownloader::Initialize();

	while(true){
		int option_index = 0;
		int c = getopt_long(argc, argv, "",long_options, &option_index);
		if (c == -1)
			break;
		std::list<IDownload>* list;
		switch(c){
			case RAPID_DOWNLOAD:{
				list=rapidDownload->search(optarg);
				rapidDownload->download(*list);
				break;
			}
			case RAPID_VALIDATE:{
				int res=fileSystem->validatePool(fileSystem->getSpringDir()+"/pool/");
				printf("Validated %d files",res);
				break;
			}
			case RAPID_LIST:{
				std::list<IDownload>* list=rapidDownload->search("");
				std::list<IDownload>::iterator it;
				for(it=list->begin();it!=list->end();++it){
					printf("%s %s\n",(*it).url.c_str(), (*it).name.c_str());
				}
				delete(list);
				break;
			}
			case PLASMA_SEARCH:{
				std::string tmp=optarg;
				plasmaDownload->search(tmp);
			}
			case PLASMA_DOWNLOAD: {
				list=plasmaDownload->search(optarg);
				plasmaDownload->download(*list);
				break;
			}
			case TORRENT_DOWNLOAD: {
				list=torrentDownload->search(optarg);
				torrentDownload->download(*list);
				break;
			}
			case WIDGET_SEARCH: {
				std::string tmp=optarg;
				widgetDownload->search(tmp);
				break;
			}
			case FILESYSTEM_WRITEPATH: {
				printf("%s\n",fileSystem->getSpringDir().c_str());
				break;
			}
			case HTTP_SEARCH:{
				std::list<IDownload>* list=httpDownload->search(optarg);
				if (list==NULL)
					break;
				std::list<IDownload>::iterator it;
				for(it=list->begin();it!=list->end();++it){
					printf("%s %s\n",(*it).url.c_str(), (*it).name.c_str());
				}
				break;
			}
			case HTTP_DOWNLOAD: {
				list=httpDownload->search(optarg);
				httpDownload->download(*list);
				break;
			}
			case DOWNLOAD_MAP:{
				download(optarg, IDownload::CAT_MAPS);
				break;
			}
			case DOWNLOAD_MOD:{
				download(optarg, IDownload::CAT_MODS);
				break;
			}
			case HELP:
			default: {
				show_help(argv[0]);
				break;
			}
		}
	}
	IDownloader::Shutdown();
	CFileSystem::Shutdown();

    return 0;
}


