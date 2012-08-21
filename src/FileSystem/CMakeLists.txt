add_library(FileSystem
		"AtomicFile.cpp"
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
	pr-md5
	pr-sha1
	pr-7z
	bencode
)

