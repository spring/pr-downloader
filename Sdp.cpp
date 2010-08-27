#include "RepoMaster.h"
#include "Sdp.h"
#include "HttpDownload.h"
#include "FileSystem.h"
#include "Util.h"
#include <string>
#include <stdio.h>

CSdp::CSdp(const std::string& shortname, const std::string& md5, const std::string& name, const std::string& url){
	this->shortname=shortname;
	this->name=name;
	this->md5=md5;
	this->url=url;
	this->downloaded=false;
}
/*
	download a mod, we already know the host where to download from + the md5 of the sdp file
	we have to download the sdp + parse it + download associated files
*/
void CSdp::download(){
	if(downloaded) //allow download only once of the same sdp
		return;
	filename=fileSystem->getSpringDir();
	filename.append( "/packages/");
	filename  += md5 + ".sdp";
	httpDownload->download(url + "/packages/" + md5 + ".sdp", filename);
	std::list<CFileSystem::FileData*>* files=fileSystem->parseSdp(filename);
	std::list<CFileSystem::FileData*>::iterator it;

	for(it=files->begin(); it!=files->end(); ++it){
		std::string springdir=fileSystem->getSpringDir();
		std::string relfile="";
		std::string filePath;
		relfile.append( "/pool/");
		md5ItoA((*it)->md5, filePath);
		relfile += filePath.at(0);
		relfile +=filePath.at(1);
		relfile.append("/");
		relfile.append(filePath.substr(2));
		relfile += ".gz";
		filePath=springdir+relfile;
		if (!fileSystem->fileIsValid(*it,filePath)){
			std::string tmpurl;
			tmpurl=url+relfile;
			httpDownload->download(tmpurl,filePath);
		}
	}
	delete(files);
	downloaded=true;
}

const std::string& CSdp::getMD5(){
	return md5;
}

const std::string& CSdp::getName(){
	return name;
}
const std::string& CSdp::getShortName(){
	return shortname;
}
