add_library(Plasma
	PlasmaDownloader.cpp
	soap/soapC.cpp
	soap/soapContentServiceSoap12Proxy.cpp
	soap/soapContentServiceSoapProxy.cpp
	soap/stdsoap2.cpp
)

target_link_libraries(Plasma
	${WIN32LIBS}
)

