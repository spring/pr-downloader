
find_package(CURL)

if (CURL_FOUND)
add_library(Http "HttpDownloader.cpp"
xmlrpc++/src/
xmlrpc++/src/XmlRpcServerMethod.h
xmlrpc++/src/XmlRpcSource.h
xmlrpc++/src/XmlRpcUtil.cpp
xmlrpc++/src/XmlRpcServer.cpp
xmlrpc++/src/XmlRpcSocket.cpp
xmlrpc++/src/Doxyfile
xmlrpc++/src/XmlRpcServer.h
xmlrpc++/src/XmlRpcUtil.h
xmlrpc++/src/XmlRpcServerConnection.cpp
xmlrpc++/src/XmlRpcValue.h
xmlrpc++/src/XmlRpcServerConnection.h
xmlrpc++/src/XmlRpcServerMethod.cpp
xmlrpc++/src/base64.h
xmlrpc++/src/XmlRpcException.h
xmlrpc++/src/XmlRpc.h
xmlrpc++/src/XmlRpcSocket.h
xmlrpc++/src/XmlRpcClient.h
xmlrpc++/src/XmlRpcClient.cpp
xmlrpc++/src/XmlRpcDispatch.h
xmlrpc++/src/XmlRpcValue.cpp
xmlrpc++/src/XmlRpcSource.cpp
xmlrpc++/src/XmlRpcDispatch.cpp
)
endif()

