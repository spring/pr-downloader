remove_definitions(-DDEBUG)

add_library(soap
	soapC.cpp
	soapContentServiceSoap12Proxy.cpp
	stdsoap2.cpp
)
target_link_libraries(soap
	${WIN32LIBS}
)

