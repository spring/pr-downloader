
#include "lib/soap/soapContentServiceSoap12Proxy.h"
#include "lib/soap/ContentServiceSoap.nsmap"

#include "PlasmaDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Downloader/Download.h"



CPlasmaDownloader::CPlasmaDownloader()
{
	this->torrentPath=fileSystem->getSpringDir()+PATH_DELIMITER +  "torrent" + PATH_DELIMITER;
	fileSystem->createSubdirs(this->torrentPath);
}

bool CPlasmaDownloader::search(std::list<IDownload*>& result, const std::string& name, IDownload::category category)
{
	LOG_DEBUG("%s",name.c_str());
	ContentServiceSoap12Proxy service;
	_ns1__DownloadFile file;
	_ns1__DownloadFileResponse fileResponse;
	std::string tmpname=name;
	file.internalName=&tmpname;
	int res;
	res=service.DownloadFile(&file, &fileResponse);
	if (res != SOAP_OK) {
		LOG_ERROR("Soap error: %d: %s\n",res, service.soap_fault_string());
		return NULL;
	}
	if (!fileResponse.DownloadFileResult) {
		LOG_ERROR("No file found for criteria %s\n",name.c_str());
		return NULL;
	}

	std::vector<std::string>::iterator it;
	IDownload::category cat=category;
	std::string fileName=fileSystem->getSpringDir() + PATH_DELIMITER;
	switch (fileResponse.resourceType) {
	case ns1__ResourceType__Map:
		cat=IDownload::CAT_MAPS;
		fileName.append("maps");
		break;
	case ns1__ResourceType__Mod:
		cat=IDownload::CAT_MODS;
		fileName.append("games");
		break;
	default:
		LOG_DEBUG("Unknown category in result: %d\n", cat);
		cat=IDownload::CAT_NONE;
		break;
	}
	fileName+=PATH_DELIMITER;
	if (fileResponse.links->string.size()==0) {
		LOG_ERROR("No mirror in plasma result.\n");
		return false;
	}

	std::string torrent;
	torrent.assign((char*)fileResponse.torrent->__ptr,fileResponse.torrent->__size);
	IDownload* dl = new IDownload();
	//parse torrent data and fill set values inside dl
	fileSystem->parseTorrent((char*)fileResponse.torrent->__ptr, fileResponse.torrent->__size, dl);

	//set full path name
	fileName.append(dl->name);
	dl->name=fileName;
	dl->cat=cat;
	LOG_DEBUG("Got filename \"%s\" from torrent\n",fileName.c_str());

	for (it=fileResponse.links->string.begin(); it!=fileResponse.links->string.end(); ++it) {
		dl->addMirror((*it).c_str());
	}
	for (it=fileResponse.dependencies->string.begin(); it!=fileResponse.dependencies->string.end(); ++it) {
		dl->addDepend((*it).c_str());
	}
	result.push_back(dl);
	return true;
}

bool CPlasmaDownloader::download(IDownload* download)
{
	LOG_DEBUG("%s",download->name.c_str());
	return httpDownload->download(download);
}
