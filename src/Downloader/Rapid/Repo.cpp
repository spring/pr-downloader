#include "RepoMaster.h"
#include "Repo.h"
#include "../../FileSystem.h"
#include "../IDownloader.h"
#include "RapidDownloader.h"
#include "Sdp.h"
#include "../../Util.h"
#include <zlib.h>
#include <stdio.h>


void CRepo::download(){
	std::string tmp;
	urlToPath(repourl,tmp);
	DEBUG_LINE("%s",tmp.c_str());
	this->tmpFile = fileSystem->getSpringDir() + PATH_DELIMITER + "rapid" + PATH_DELIMITER +tmp + PATH_DELIMITER +"versions.gz";
	fileSystem->createSubdirs(tmpFile);
	if (fileSystem->isOlder(tmpFile,REPO_RECHECK_TIME))
		if (parse()) //first try already downloaded file, as repo master file rarely changes
			return;
	fileSystem->createSubdirs(tmpFile);
	IDownload dl(repourl + "/versions.gz", tmpFile);
	httpDownload->download(dl);
	parse();
}

bool CRepo::parse(){
	DEBUG_LINE("%s",tmpFile.c_str());
	gzFile fp=gzopen(tmpFile.c_str(), "r");
	if (fp==Z_NULL){
		printf("Could not open %s\n",tmpFile.c_str());
		return false;
	}
	char buf[1024];
	sdps.empty();
	while ((gzgets(fp, buf, sizeof(buf)))!=Z_NULL){
		for (unsigned int i=0;i<sizeof(buf);i++){
			if (buf[i]=='\n'){
				buf[i]=0;
				break;
			}
		}

		std::string tmp=buf;
		std::string shortname=getStrByIdx(tmp,',',0);
		std::string md5=getStrByIdx(tmp,',',1);
		std::string depends=getStrByIdx(tmp,',',2);
		std::string name=getStrByIdx(tmp,',',3);
		if (shortname.size()>0){ //create new repo from url
			CSdp* sdptmp=new CSdp(shortname, md5, name, depends, repourl);
			rapid->addRemoteDsp(sdptmp);
		}
	}
	gzclose(fp);
	return true;
}
