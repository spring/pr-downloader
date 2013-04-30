/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "PlasmaDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Downloader/Download.h"
#include "Logger.h"
#include "lib/soap/soapContentServiceSoap12Proxy.h"
#include "lib/soap/ContentServiceSoap.nsmap"


CPlasmaDownloader::CPlasmaDownloader():
	torrentPath(fileSystem->getSpringDir()+PATH_DELIMITER +  "torrent" + PATH_DELIMITER)
{
	fileSystem->createSubdirs(this->torrentPath);
}

bool CPlasmaDownloader::search(std::list<IDownload*>& result, const std::string& name, IDownload::category category)
{
	LOG_DEBUG("%s",name.c_str());
	ContentServiceSoap12Proxy service;
	_Plasma__DownloadFile file;
	_Plasma__DownloadFileResponse fileResponse;
	std::string tmpname=name;
	file.internalName=&tmpname;
	int res;
	res=service.DownloadFile(&file, &fileResponse);
	if (res != SOAP_OK) {
		LOG_ERROR("Soap error: %d: %s",res, service.soap_fault_string());
		return false;
	}
	if (!fileResponse.DownloadFileResult) {
		LOG_DEBUG("No file found for criteria %s",name.c_str());
		return false;
	}

	std::vector<std::string>::iterator it;
	IDownload::category cat=category;
	std::string fileName=fileSystem->getSpringDir() + PATH_DELIMITER;
	switch (fileResponse.resourceType) {
	case Plasma__ResourceType__Map:
		cat=IDownload::CAT_MAPS;
		fileName.append("maps");
		break;
	case Plasma__ResourceType__Mod:
		cat=IDownload::CAT_GAMES;
		fileName.append("games");
		break;
	default:
		LOG_DEBUG("Unknown category in result: %d", cat);
		cat=IDownload::CAT_NONE;
		break;
	}
	fileName+=PATH_DELIMITER;
	if (fileResponse.links->string.size()==0) {
		LOG_DEBUG("No mirror in plasma result.");
		return false;
	}

	std::string torrent;
	torrent.assign((char*)fileResponse.torrent->__ptr,fileResponse.torrent->__size);
	IDownload* dl = new IDownload();
	//parse torrent data and fill set values inside dl
	const bool bres = fileSystem->parseTorrent((char*)fileResponse.torrent->__ptr, fileResponse.torrent->__size, dl);
	if ( (dl->name == "") || (!bres)) {
		LOG_ERROR("Couldn't parse torrent filename");
		return false;
	}

	//set full path name
	fileName.append(dl->name);
	dl->name=fileName;
	dl->cat=cat;
	LOG_DEBUG("Got filename \"%s\" from torrent",fileName.c_str());

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
