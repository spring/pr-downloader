if (PRD_ARCHIVE_SUPPORT)
	set(archivessrc SevenZipArchive.cpp ZipArchive.cpp)
	set(archiveslib pr-7z)
	add_definitions(-DARCHIVE_SUPPORT)
endif()

add_library(FileSystem
		AtomicFile.cpp
		FileData.cpp
		FileSystem.cpp
		File.cpp
		HashMD5.cpp
		HashSHA1.cpp
		HashCRC32.cpp
		IHash.cpp
		${archivessrc}
)
target_link_libraries(FileSystem
	${ZLIB_LIBRARIES}
	pr-md5
	pr-sha1
	bencode
	${archiveslib}
)
if(PRD_ARCHIVE_SUPPORT)
	if (MINIZIP_FOUND)
		target_link_libraries(FileSystem ${MINIZIP_LIBRARIES})
	else()
		target_link_libraries(FileSystem pr-minizip)
	endif()
endif()

