all:
	wsdl2h -o plasma.h http://zero-k.info/ContentService.asmx?WSDL
	soapcpp2 -i -C -I/usr/include/gsoap/ plasma.h
clean:
	rm -rf \
	ContentServiceSoap12.DownloadFile.req.xml \
	ContentServiceSoap12.DownloadFile.res.xml \
	ContentServiceSoap12.GetResourceData.req.xml \
	ContentServiceSoap12.GetResourceData.res.xml \
	ContentServiceSoap12.GetResourceList.req.xml \
	ContentServiceSoap12.GetResourceList.res.xml \
	ContentServiceSoap12.GetScriptMissionData.req.xml \
	ContentServiceSoap12.GetScriptMissionData.res.xml \
	ContentServiceSoap12.NotifyMissionRun.req.xml \
	ContentServiceSoap12.NotifyMissionRun.res.xml \
	ContentServiceSoap12.nsmap \
	ContentServiceSoap12.RegisterResource.req.xml \
	ContentServiceSoap12.RegisterResource.res.xml \
	ContentServiceSoap12.SubmitMissionScore.req.xml \
	ContentServiceSoap12.SubmitMissionScore.res.xml \
	ContentServiceSoap12.SubmitStackTrace.req.xml \
	ContentServiceSoap12.SubmitStackTrace.res.xml \
	ContentServiceSoap.DownloadFile.req.xml \
	ContentServiceSoap.DownloadFile.res.xml \
	ContentServiceSoap.GetResourceData.req.xml \
	ContentServiceSoap.GetResourceData.res.xml \
	ContentServiceSoap.GetResourceList.req.xml \
	ContentServiceSoap.GetResourceList.res.xml \
	ContentServiceSoap.GetScriptMissionData.req.xml \
	ContentServiceSoap.GetScriptMissionData.res.xml \
	ContentServiceSoap.NotifyMissionRun.req.xml \
	ContentServiceSoap.NotifyMissionRun.res.xml \
	ContentServiceSoap.nsmap \
	ContentServiceSoap.RegisterResource.req.xml \
	ContentServiceSoap.RegisterResource.res.xml \
	ContentServiceSoap.SubmitMissionScore.req.xml \
	ContentServiceSoap.SubmitMissionScore.res.xml \
	ContentServiceSoap.SubmitStackTrace.req.xml \
	ContentServiceSoap.SubmitStackTrace.res.xml \
	plasma.h \
	soapC.cpp \
	soapContentServiceSoap12Proxy.cpp \
	soapContentServiceSoap12Proxy.h \
	soapContentServiceSoapProxy.cpp \
	soapContentServiceSoapProxy.h \
	soapH.h \
	soapStub.h
