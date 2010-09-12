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
	this->tmpFile = fileSystem->getSpringDir() + PATH_DELIMITER + "rapid" + PATH_DELIMITER +tmp + PATH_DELIMITER +"versions.gz";
	fileSystem->createSubdirs(tmpFile);
	httpDownload->addDownload(repourl + "/versions.gz", tmpFile);
	httpDownload->start();
	parse();
}


void CRepo::parse(){
	gzFile fp=gzopen(tmpFile.c_str(), "r");
	if (fp==Z_NULL){
        printf("Could not open %s\n",tmpFile.c_str());
		return;
	}
    char buf[4096];
	sdps.empty();
    while(gzgets(fp, buf, sizeof(buf))!=Z_NULL){
    	for(unsigned int i=0;i<sizeof(buf);i++){
    		if(buf[i]=='\n'){
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
			CRapidDownloader* tmp=(CRapidDownloader*)rapidDownload;
			tmp->addRemoteDsp(*sdptmp);
		}
    }
    gzclose(fp);
}
