


#define VERSION 0.1
#define USER_AGENT "unitsync-dev" + VERSION
#define SPRING_DIR "/home/matze/.spring"
#define TMP_FILE "/tmp/repos.gz"
#define TMP_FILE2 "/tmp/version.gz"


#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "md5.h"
#include "RapidDownloader.h"
#include "Util.h"
#include <string>


int main(int argc, char **argv){
	rapidDownloader->Initialize();
//	rapidDownloader->list_tag();
	rapidDownloader->download_tag("ba:latest");
	rapidDownloader->Shutdown();

    return 0;
}


