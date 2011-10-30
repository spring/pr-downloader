add_library(soap
	soapC.cpp
	soapContentServiceSoap12Proxy.cpp
	soapContentServiceSoapProxy.cpp
	stdsoap2.cpp
)
target_link_libraries(soap
	${WIN32LIBS}
)

