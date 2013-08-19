add_library(xmlrpc STATIC
	src/XmlRpcUtil.cpp
	src/XmlRpcSocket.cpp
	src/XmlRpcClient.cpp
	src/XmlRpcCurlClient.cpp
	src/XmlRpcDispatch.h
	src/XmlRpcValue.cpp
	src/XmlRpcSource.cpp
	src/XmlRpcDispatch.cpp
)

