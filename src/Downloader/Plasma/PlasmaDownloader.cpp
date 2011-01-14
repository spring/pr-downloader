#include "soap/soapContentServiceSoap12Proxy.h"
#include "soap/ContentServiceSoap.nsmap"
#include "PlasmaDownloader.h"
#include "../../FileSystem.h"
#include "../../Util.h"



CPlasmaDownloader::CPlasmaDownloader(){
	this->torrentPath=fileSystem->getSpringDir()+PATH_DELIMITER +  "torrent" + PATH_DELIMITER;
	fileSystem->createSubdirs(this->torrentPath);
}

std::list<IDownload>* CPlasmaDownloader::search(const std::string& name, IDownload::category category){
	DEBUG_LINE("%s",name.c_str());
	ContentServiceSoap12Proxy service;
	_ns1__DownloadFile file;
	_ns1__DownloadFileResponse result;
	std::string tmpname=name;
	file.internalName=&tmpname;
	std::list<IDownload>* dlres;
	int res;
	res=service.DownloadFile(&file, &result);
	if (res != SOAP_OK){
		printf("Soap error: %d: %s\n",res, service.soap_fault_string());
		return NULL;
	}
	if (!result.DownloadFileResult){
		printf("No file found for criteria %s\n",name.c_str());
		return NULL;
	}
	std::string fileName=this->torrentPath;
	fileName.append(*result.torrentFileName);


	DEBUG_LINE("Saving torrent to %s",fileName.c_str());
	xsd__base64Binary *torrent_buf=result.torrent;
	FILE* f=fopen(fileName.c_str(),"wb");
	fwrite(torrent_buf->__ptr, torrent_buf->__size, 1, f);
	fclose(f);

	std::vector<std::string>::iterator it;
	dlres=new std::list<IDownload>();

	std::string saveto=fileSystem->getSpringDir();
	IDownload::category cat;
	if (result.resourceType==ns1__ResourceType__Map){
		saveto += PATH_DELIMITER;
		saveto += "maps";
		saveto += PATH_DELIMITER;
		cat=IDownload::CAT_MAPS;
	}else{
		saveto += PATH_DELIMITER;
		saveto += "games";
		saveto += PATH_DELIMITER;
		cat=IDownload::CAT_MODS;
	}
	DEBUG_LINE("Saving file to %s",saveto.c_str());
	IDownload* dl=new IDownload(fileName,saveto,cat);
	for(it=result.links->string.begin();it!=result.links->string.end(); ++it){
		dl->addMirror((*it).c_str());
	}
	for(it=result.dependencies->string.begin();it!=result.dependencies->string.end(); ++it){
		dl->addDepend((*it).c_str());
	}
	dlres->push_back(*dl);
	return dlres;
}

bool CPlasmaDownloader::download(IDownload& download){
	DEBUG_LINE("%s",download.name.c_str());
	return torrentDownload->download(download);
}
