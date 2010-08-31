all:
	wsdl2h -o plasma.h http://planet-wars.eu/PlasmaServer/Service.asmx?WSDL
	soapcpp2 -i -C -I/usr/include/gsoap/ plasma.h
clean:
	rm -rf \
	PlasmaServiceSoap12.DeleteResource.req.xml \
	PlasmaServiceSoap12.DeleteResource.res.xml \
	PlasmaServiceSoap12.DownloadFile.req.xml \
	PlasmaServiceSoap12.DownloadFile.res.xml \
	PlasmaServiceSoap12.GetResourceData.req.xml \
	PlasmaServiceSoap12.GetResourceData.res.xml \
	PlasmaServiceSoap12.GetResourceList.req.xml \
	PlasmaServiceSoap12.GetResourceList.res.xml \
	PlasmaServiceSoap12.nsmap \
	PlasmaServiceSoap12.RegisterResource.req.xml \
	PlasmaServiceSoap12.RegisterResource.res.xml \
	PlasmaServiceSoap.DeleteResource.req.xml \
	PlasmaServiceSoap.DeleteResource.res.xml \
	PlasmaServiceSoap.DownloadFile.req.xml \
	PlasmaServiceSoap.DownloadFile.res.xml \
	PlasmaServiceSoap.GetResourceData.req.xml \
	PlasmaServiceSoap.GetResourceData.res.xml \
	PlasmaServiceSoap.GetResourceList.req.xml \
	PlasmaServiceSoap.GetResourceList.res.xml \
	PlasmaServiceSoap.nsmap \
	PlasmaServiceSoap.RegisterResource.req.xml \
	PlasmaServiceSoap.RegisterResource.res.xml \
	plasma.h \
	soapH.h \
	soapPlasmaServiceSoap12Object.h \
	soapPlasmaServiceSoap12Proxy.h \
	soapPlasmaServiceSoapObject.h \
	soapPlasmaServiceSoapProxy.h \
	soapStub.h \
	soapC.cpp \
	soapClient.cpp \
	soapClientLib.cpp \
	soapServer.cpp \
	soapServerLib.cpp 

