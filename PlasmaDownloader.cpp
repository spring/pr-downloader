#include "plasma/soapPlasmaServiceSoap12Proxy.h"
#include "plasma/PlasmaServiceSoap.nsmap"
#include "PlasmaDownloader.h"



CPlasmaDownloader::CPlasmaDownloader(){
}

void CPlasmaDownloader::download(){
//	struct soap *soap = soap_new();
	PlasmaServiceSoap12Proxy service;
	_ns1__DownloadFile file;
	_ns1__DownloadFileResponse result;

	std::string name="BA715.sd7";
	file.internalName=&name;

	if (service.DownloadFile(&file, &result) == SOAP_OK)
		if (result.DownloadFileResult)
      		printf("download ok\n");
		else
			printf("download failed\n");
	else
      printf("soap!=ok\n");
}


