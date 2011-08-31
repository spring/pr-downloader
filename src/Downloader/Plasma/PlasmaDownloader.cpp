
#include "soap/soapContentServiceSoap12Proxy.h"
#include "soap/ContentServiceSoap.nsmap"

#include "PlasmaDownloader.h"
#include "../../FileSystem.h"
#include "../../Util.h"
#include "bencode/bencode.h"
#include "pr-downloader/Download.h"

void CPlasmaDownloader::parseTorrent(char*data, int size){
/*
extern be_node *be_decode(const char *bencode);
extern be_node *be_decoden(const char *bencode, long long bencode_len);
extern void be_free(be_node *node);
extern void be_dump(be_node *node);
*/

	struct be_node* node=be_decoden(data, size);
	be_dump(node);
}



CPlasmaDownloader::CPlasmaDownloader(){
	this->torrentPath=fileSystem->getSpringDir()+PATH_DELIMITER +  "torrent" + PATH_DELIMITER;
	fileSystem->createSubdirs(this->torrentPath);
}

bool CPlasmaDownloader::search(std::list<IDownload>& result, const std::string& name, IDownload::category category){
	DEBUG_LINE("%s",name.c_str());
	ContentServiceSoap12Proxy service;
	_ns1__DownloadFile file;
	_ns1__DownloadFileResponse fileResponse;
	std::string tmpname=name;
	file.internalName=&tmpname;
	int res;
	res=service.DownloadFile(&file, &fileResponse);
	if (res != SOAP_OK){
		printf("Soap error: %d: %s\n",res, service.soap_fault_string());
		return NULL;
	}
	if (!fileResponse.DownloadFileResult){
		printf("No file found for criteria %s\n",name.c_str());
		return NULL;
	}

	std::vector<std::string>::iterator it;
	IDownload::category cat=category;
	std::string fileName=fileSystem->getSpringDir() + PATH_DELIMITER;
	switch (fileResponse.resourceType){
		case ns1__ResourceType__Map:
			cat=IDownload::CAT_MAPS;
			fileName.append("maps");
			break;
		case ns1__ResourceType__Mod:
			cat=IDownload::CAT_MODS;
			fileName.append("games");
			break;
		default:
			DEBUG_LINE("Unknown category in result: %d\n", cat);
			cat=IDownload::CAT_NONE;
			break;
	}
	fileName+=PATH_DELIMITER;
	if (fileResponse.links->string.size()==0){
		printf("got no mirror in plasmaresoult\n");
		return false;
	}

	std::string torrent;
	torrent.assign((char*)fileResponse.torrent->__ptr,fileResponse.torrent->__size);
	parseTorrent((char*)fileResponse.torrent->__ptr, fileResponse.torrent->__size);
//	simple .torrent parser to get filename: need to parse for example
// :name27:Tech Annihilation v1.08.sd7:
// -> search :name, search next ":" convert to int, read name

	int pos=torrent.find(":name"); //TODO: this is ugly + to hardcoded...
	int end=torrent.find(":",pos+1);
	int len=atoi(torrent.substr(pos+5,end-pos-5).c_str());
	fileName.append(torrent.substr(end+1,len));

	DEBUG_LINE("Got filename \"%s\" from torrent\n",fileName.c_str());

	IDownload dl;
	for (it=fileResponse.links->string.begin();it!=fileResponse.links->string.end(); ++it){
		dl=IDownload(fileName,cat);
		dl.addMirror((*it).c_str());
	}
	for (it=fileResponse.dependencies->string.begin();it!=fileResponse.dependencies->string.end(); ++it){
		dl.addDepend((*it).c_str());
	}
	result.push_back(dl);
	return true;
}

bool CPlasmaDownloader::download(IDownload& download){
	DEBUG_LINE("%s",download.name.c_str());
	return httpDownload->download(download);
}
