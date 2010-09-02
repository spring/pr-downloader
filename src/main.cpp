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



int main(int argc, char **argv){
	enum{
		RAPID_DOWNLOAD=0,
		RAPID_VALIDATE,
		RAPID_LIST,
		PLASMA_DOWNLOAD,
		TORRENT_DOWNLOAD,
		HELP
	};

	static struct option long_options[] = {
		{"rapid-download"   , 1, 0, RAPID_DOWNLOAD},
		{"rapid-validate"   , 0, 0, RAPID_VALIDATE},
		{"rapid-list"       , 0, 0, RAPID_LIST},
		{"plasma-download"  , 1, 0, PLASMA_DOWNLOAD},
		{"torrent-download" , 1, 0, TORRENT_DOWNLOAD},
		{"help"             , 0, 0, HELP},
		{0                  , 0, 0, 0}
	};
	CFileSystem::Initialize();
	IDownloader::Initialize();
	while(true){
		int option_index = 0;
		int c = getopt_long(argc, argv, "",long_options, &option_index);
		if (c == -1)
			break;
		switch(c){
			case RAPID_DOWNLOAD:{
				rapidDownload.addDownload(optarg);
				break;
			}
			case RAPID_VALIDATE:{
				fileSystem->validatePool(fileSystem->getSpringDir()+"/pool/");
				break;
			}
			case RAPID_LIST:{
				rapidDownload.search(optarg);
				break;
			}
			case PLASMA_DOWNLOAD: {
				plasmaDownload.addDownload(optarg);
				plasmaDownload.start();
				break;
			}
			case TORRENT_DOWNLOAD: {
				torrentDownload.addDownload(optarg);
				torrentDownload.start();
			}
			case HELP:
			default:{
				int i=0;
				printf("Usage: %s ", argv[0]);
				bool append=false;
				while(long_options[i].name!=0){
					if (append){
						printf("|");
					}
					append=true;
					printf("--%s",long_options[i].name);
					if (long_options[i].has_arg!=0)
						printf(" <name>");
					i++;
				}
				printf("\n");
				exit(1);
				break;
			}
		}
	}
	IDownloader::Shutdown();
	CFileSystem::Shutdown();

    return 0;
}


