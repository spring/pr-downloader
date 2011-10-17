#include "RepoMaster.h"
#include "Repo.h"
#include "FileSystem/FileSystem.h"
#include "Downloader/IDownloader.h"
#include "RapidDownloader.h"
#include "Sdp.h"
#include "Util.h"
#include <zlib.h>
#include <stdio.h>


CRepo::CRepo(const std::string& repourl, CRapidDownloader* rapid)
{
	this->repourl=repourl;
	this->rapid=rapid;
}

CRepo::~CRepo()
{

}

void CRepo::download()
{
	std::string tmp;
	urlToPath(repourl,tmp);
	LOG_DEBUG("%s",tmp.c_str());
	this->tmpFile = fileSystem->getSpringDir() + PATH_DELIMITER + "rapid" + PATH_DELIMITER +tmp + PATH_DELIMITER +"versions.gz";
	fileSystem->createSubdirs(tmpFile);
	if (fileSystem->isOlder(tmpFile,REPO_RECHECK_TIME))
		if (parse()) //first try already downloaded file, as repo master file rarely changes
			return;
	fileSystem->createSubdirs(tmpFile);
	IDownload dl(tmpFile);
	dl.addMirror(repourl + "/versions.gz");
	httpDownload->download(&dl);
	parse();
}

bool CRepo::parse()
{
	LOG_DEBUG("%s",tmpFile.c_str());
	gzFile fp=gzopen(tmpFile.c_str(), "r");
	if (fp==Z_NULL) {
		LOG_ERROR("Could not open %s\n",tmpFile.c_str());
		return false;
	}

	char buf[IO_BUF_SIZE];
	sdps.empty();
	while ((gzgets(fp, buf, sizeof(buf)))!=Z_NULL) {
		for (unsigned int i=0; i<sizeof(buf); i++) {
			if (buf[i]=='\n') {
				buf[i]=0;
				break;
			}
		}

		std::string tmp=buf;
		std::string shortname=getStrByIdx(tmp,',',0);
		std::string md5=getStrByIdx(tmp,',',1);
		std::string depends=getStrByIdx(tmp,',',2);
		std::string name=getStrByIdx(tmp,',',3);
		if (shortname.size()>0) { //create new repo from url
			CSdp sdptmp=CSdp(shortname, md5, name, depends, repourl);
			rapid->addRemoteDsp(sdptmp);
		}
	}
	gzclose(fp);
	return true;
}
