
add_library(Http STATIC
	HttpDownloader.cpp
	DownloadData.cpp
)
target_link_libraries(Http
	${CURL_LIBRARIES}
	${OPENSSL_LIBRARIES}
	xmlrpc
)

