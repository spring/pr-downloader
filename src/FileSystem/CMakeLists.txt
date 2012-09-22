add_library(FileSystem
		AtomicFile.cpp
		FileData.cpp
		FileSystem.cpp
		File.cpp
		HashMD5.cpp
		HashSHA1.cpp
		HashCRC32.cpp
		IHash.cpp
)

FIND_PACKAGE(LibArchive REQUIRED)

target_link_libraries(FileSystem
	${ZLIB_LIBRARIES}
	${LibArchive_LIBRARIES}
	${LZMA_LIBRARIES}
	pr-md5
	pr-sha1
	bencode
)

