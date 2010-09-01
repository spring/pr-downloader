#include "plasma/soapPlasmaServiceSoap12Proxy.h"
#include "plasma/PlasmaServiceSoap.nsmap"
#include "PlasmaDownloader.h"



CPlasmaDownloader::CPlasmaDownloader(){
}

void CPlasmaDownloader::download(const std::string& name){
//	struct soap *soap = soap_new();
	PlasmaServiceSoap12Proxy service;
	_ns1__DownloadFile file;
	_ns1__DownloadFileResponse result;
	std::string tmpname=name;
	file.internalName=&tmpname;

	if (service.DownloadFile(&file, &result) == SOAP_OK)
		if (result.DownloadFileResult){
			printf("download ok\n");
			std::string *torrent=result.torrentFileName;
			std::vector<std::string>* deps=&result.dependencies->string;
			std::vector<std::string>* links=&result.links->string;

			printf("%s\n",torrent->c_str());
			xsd__base64Binary *torrent_buf=result.torrent;
			FILE* f=fopen(torrent->c_str(),"wb");
			fwrite(torrent_buf->__ptr, torrent_buf->__size, 1, f);
			fclose(f);

			std::vector<std::string>::iterator it;
			for(it=result.links->string.begin();it!=result.links->string.end(); it++){
				printf("%s\n",(*it).c_str());
			}
			for(it=result.dependencies->string.begin();it!=result.dependencies->string.end(); it++){
				printf("%s\n",(*it).c_str());
			}
		}else
			printf("download failed\n");
	else
      printf("soap!=ok\n");
}


