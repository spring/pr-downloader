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
	filename=fileSystem->getSpringDir() + "/packages/";
	if (!fileSystem->directory_exist(filename)){
		fileSystem->create_subdirs(filename);
	}
	filename  += md5 + ".sdp";
	httpDownload->download(url + "/packages/" + md5 + ".sdp", filename);
	std::list<CFileSystem::FileData*>* files=fileSystem->parseSdp(filename);
	std::list<CFileSystem::FileData*>::iterator it;
	httpDownload->setCount(files->size());
	int i=0;
	for(it=files->begin(); it!=files->end(); ++it){
		i++;
		printf("%d/%d\r",i,(unsigned int)files->size());
		std::string md5="";

		md5ItoA((*it)->md5, md5);
		std::string filename=md5.substr(2);
		filename.append(".gz");
		std::string path("/pool/");
		path += md5.at(0);
		path += md5.at(1);
		path += "/";

		std::string file=fileSystem->getSpringDir() + path + filename; //absolute filename
		std::string url=this->url + path + filename; //absolute url

		if (!fileSystem->directory_exist(fileSystem->getSpringDir()+path)){
			fileSystem->create_subdirs(fileSystem->getSpringDir()+path);
		}
		if (!fileSystem->fileIsValid(*it,file)){
			httpDownload->download(url,file,i);
			if (!fileSystem->fileIsValid(*it,file)){
				printf("Error downloading %s file md5 is invalid\n",url.c_str());
			}
		}
	}
	delete(files);
	printf("Succesfully downloaded %s %s\n",shortname.c_str(),name.c_str());
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
