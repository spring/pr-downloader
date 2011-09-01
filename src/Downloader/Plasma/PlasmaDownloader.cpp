
#include "soap/soapContentServiceSoap12Proxy.h"
#include "soap/ContentServiceSoap.nsmap"

#include "PlasmaDownloader.h"
#include "../../FileSystem.h"
#include "../../Util.h"
#include "bencode/bencode.h"
#include "pr-downloader/Download.h"

bool CPlasmaDownloader::parseTorrent(char*data, int size, IDownload& dl)
{
	struct be_node* node=be_decoden(data, size);
#ifdef DEBUG
	be_dump(node);
#endif
	if (node->type!=BE_DICT) {
		printf("Error in torrent data\n");
		return false;
	}
	int i;
	struct be_node* infonode=NULL;
	for (i = 0; node->val.d[i].val; ++i) { //search for a dict with name info
		if ((node->type==BE_DICT) && (strcmp(node->val.d[i].key,"info")==0)) {
			infonode=node->val.d[i].val;
			break;
		}
	}
	if (infonode==NULL) {
		printf("couldn't find info node in be dict\n");
		return false;
	}
	for (i = 0; infonode->val.d[i].val; ++i) { //fetch needed data from dict and fill into dl
		struct be_node*datanode;
		datanode=infonode->val.d[i].val;
		switch(datanode->type) {
		case BE_STR: //current value is a string
			if (strcmp("name",infonode->val.d[i].key)==0) { //filename
				dl.name=datanode->val.s;
			} else if (!strcmp("pieces", infonode->val.d[i].key)) { //hash sum of a piece
				const int count = strlen(datanode->val.s)/6;
				for (int i=0; i<count; i++) {
					struct IDownload::sha1 sha;
					sha.sha[0]=datanode->val.s[i*5];
					sha.sha[1]=datanode->val.s[i*5+1];
					sha.sha[2]=datanode->val.s[i*5+2];
					sha.sha[3]=datanode->val.s[i*5+3];
					sha.sha[4]=datanode->val.s[i*5+4];
					dl.pieces.push_back(sha);
				}
			}
			break;
		case BE_INT: //current value is a int
			if (strcmp("length",infonode->val.d[i].key)==0) { //filesize
				dl.size=datanode->val.i;
			} else if (!strcmp("piece length",infonode->val.d[i].key)) { //length of a piece
				dl.piecesize=datanode->val.i;
			}
			break;
		default:
			break;
		}
	}
	DEBUG_LINE("Parsed torrent data: %s %d\n", dl.name.c_str(), dl.piecesize);
	return true;
}

CPlasmaDownloader::CPlasmaDownloader()
{
	this->torrentPath=fileSystem->getSpringDir()+PATH_DELIMITER +  "torrent" + PATH_DELIMITER;
	fileSystem->createSubdirs(this->torrentPath);
}

bool CPlasmaDownloader::search(std::list<IDownload>& result, const std::string& name, IDownload::category category)
{
	DEBUG_LINE("%s",name.c_str());
	ContentServiceSoap12Proxy service;
	_ns1__DownloadFile file;
	_ns1__DownloadFileResponse fileResponse;
	std::string tmpname=name;
	file.internalName=&tmpname;
	int res;
	res=service.DownloadFile(&file, &fileResponse);
	if (res != SOAP_OK) {
		printf("Soap error: %d: %s\n",res, service.soap_fault_string());
		return NULL;
	}
	if (!fileResponse.DownloadFileResult) {
		printf("No file found for criteria %s\n",name.c_str());
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
		DEBUG_LINE("Unknown category in result: %d\n", cat);
		cat=IDownload::CAT_NONE;
		break;
	}
	fileName+=PATH_DELIMITER;
	if (fileResponse.links->string.size()==0) {
		printf("got no mirror in plasmaresoult\n");
		return false;
	}

	std::string torrent;
	torrent.assign((char*)fileResponse.torrent->__ptr,fileResponse.torrent->__size);
	IDownload dl;
	//parse torrent data and fill set values inside dl
	parseTorrent((char*)fileResponse.torrent->__ptr, fileResponse.torrent->__size, dl);

	//set full path name
	fileName.append(dl.name);
	dl.name=fileName;
	dl.cat=cat;
	DEBUG_LINE("Got filename \"%s\" from torrent\n",fileName.c_str());

	for (it=fileResponse.links->string.begin(); it!=fileResponse.links->string.end(); ++it) {
		dl.addMirror((*it).c_str());
	}
	for (it=fileResponse.dependencies->string.begin(); it!=fileResponse.dependencies->string.end(); ++it) {
		dl.addDepend((*it).c_str());
	}
	result.push_back(dl);
	return true;
}

bool CPlasmaDownloader::download(IDownload& download)
{
	DEBUG_LINE("%s",download.name.c_str());
	return httpDownload->download(download);
}
