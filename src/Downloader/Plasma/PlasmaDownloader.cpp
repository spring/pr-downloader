#include "soap/soapPlasmaServiceSoap12Proxy.h"
#include "soap/PlasmaServiceSoap.nsmap"
#include "PlasmaDownloader.h"
#include "../../FileSystem.h"



CPlasmaDownloader::CPlasmaDownloader(){
	this->torrentPath=fileSystem->getSpringDir()+"/torrent/";
	fileSystem->create_subdirs(this->torrentPath);
}

std::list<IDownload>* CPlasmaDownloader::search(const std::string& name){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	PlasmaServiceSoap12Proxy service;
	_ns1__DownloadFile file;
	_ns1__DownloadFileResponse result;
	std::string tmpname=name;
	file.internalName=&tmpname;
	std::list<IDownload>* dlres;
	int res;
	res=service.DownloadFile(&file, &result);
	if (res != SOAP_OK){
		switch(res){
			case SOAP_TCP_ERROR:
				printf("Couldn't connect to soap-server\n");
				break;

			default:
				printf("Soap error: %d\n",res);
		}
		return NULL;
	}
	if (!result.DownloadFileResult){
		printf("No file found for criteria %s\n",name.c_str());
		return NULL;
	}
	printf("download ok\n");
	std::string fileName=this->torrentPath;
	fileName.append(*result.torrentFileName);


	printf("%s\n",fileName.c_str());
	xsd__base64Binary *torrent_buf=result.torrent;
	FILE* f=fopen(fileName.c_str(),"wb");
	fwrite(torrent_buf->__ptr, torrent_buf->__size, 1, f);
	fclose(f);

	std::vector<std::string>::iterator it;
	dlres=new std::list<IDownload>();

	std::string saveto=fileSystem->getSpringDir();
	IDownload::category cat;
	if (result.resourceType==ns1__ResourceType__Map){
		saveto.append("/maps/");
		cat=IDownload::CAT_MAPS;
	}else{
		saveto.append("/mods/");
		cat=IDownload::CAT_MODS;
	}

	IDownload* dl=new IDownload(fileName,saveto,cat);
	for(it=result.links->string.begin();it!=result.links->string.end(); it++){
		dl->addMirror((*it).c_str());
	}
	for(it=result.dependencies->string.begin();it!=result.dependencies->string.end(); it++){
		dl->addDepend((*it).c_str());
	}
	dlres->push_back(*dl);
	return dlres;
}

void CPlasmaDownloader::start(IDownload* download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	torrentDownload->start(download);
}

const IDownload* CPlasmaDownloader::addDownload(const std::string& url, const std::string& filename){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return NULL;
}
bool CPlasmaDownloader::removeDownload(IDownload& download){
	printf("%s %s:%d \n",__FILE__, __FUNCTION__ ,__LINE__);
	return true;
}
