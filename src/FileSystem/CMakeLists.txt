add_library(FileSystem
		"FileData.cpp"
		"FileSystem.cpp"
		"File.cpp"
		"HashMD5.cpp"
		"HashSHA1.cpp"
		"HashCRC32.cpp"
		"IHash.cpp"
)
target_link_libraries(FileSystem
	${ZLIB_LIBRARIES}
	md5
	sha1
	bencode
)

