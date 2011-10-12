
find_package(CURL)

if (CURL_FOUND)
add_library(Http
	HttpDownloader.cpp
)
target_link_libraries(Http
	${CURL_LIBRARIES}
	xmlrpc
)
endif()

