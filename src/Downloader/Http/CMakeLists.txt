
find_package(CURL)

if   (WIN32)
#	target_link_libraries("Http" "-Wl,-subsystem,console -lwsock32 ${MINGWLIBS}/dll/libcurl.dll")
else()
#	target_link_libraries("Http" ${CURL_LIBRARIES})
endif()

if (CURL_FOUND)
	add_library("Http" "HttpDownloader.cpp")
endif()
