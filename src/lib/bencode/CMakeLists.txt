add_library(bencode STATIC
	bencode.cpp
)
target_include_directories(bencode PRIVATE ${pr-downloader_SOURCE_DIR}/src)

