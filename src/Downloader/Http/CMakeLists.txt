
find_package(CURL)

if (CURL_FOUND)
	add_library(Http "HttpDownloader.cpp")
endif()
