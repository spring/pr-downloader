add_subdirectory(lib)

if(PRD_ARCHIVE_SUPPORT)
	set(archivessrc FileSystem/SevenZipArchive.cpp FileSystem/ZipArchive.cpp)
	set(archiveslib 7zip)
	add_definitions(-DARCHIVE_SUPPORT)
endif()

if(PRD_JSONCPP_INTERNAL)
	set(jsonlibcppsrc
		${CMAKE_CURRENT_SOURCE_DIR}/lib/jsoncpp/src/lib_json/json_value.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/lib/jsoncpp/src/lib_json/json_reader.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/lib/jsoncpp/src/lib_json/json_writer.cpp)
endif()

add_library(Downloader STATIC
	Downloader/Rapid/RapidDownloader.cpp
	Downloader/Rapid/Repo.cpp
	Downloader/Rapid/Sdp.cpp
	Downloader/Http/HttpDownloader.cpp
	Downloader/Http/DownloadData.cpp
	Downloader/CurlWrapper.cpp
	Downloader/Download.cpp
	Downloader/IDownloader.cpp
	Downloader/Mirror.cpp
	Downloader/DownloadEnum.cpp
	FileSystem/FileSystem.cpp
	FileSystem/File.cpp
	FileSystem/HashMD5.cpp
	FileSystem/HashSHA1.cpp
	FileSystem/IHash.cpp
	Util.cpp
	Version.cpp
	lib/base64/base64.cpp
	lsl/lslutils/platform.cpp
	${archivessrc}
	${jsonlibcppsrc}
)

target_include_directories(Downloader
	PRIVATE
		${pr-downloader_SOURCE_DIR}/src
		${PRD_JSONCPP_INCLUDE_DIR}
		${MINIZIP_INCLUDE_DIR}
		${CURL_INCLUDE_DIR}
		${ZLIB_INCLUDE_DIR}
)
set_source_files_properties(Version.cpp PROPERTIES COMPILE_DEFINITIONS "PR_DOWNLOADER_VERSION=${PR_DOWNLOADER_VERSION}")

if(WIN32)
	set(WIN32LIBS ws2_32 mswsock)
endif()

set(PRDOWNLOADER_LIBS
	${WIN32LIBS}
	Downloader
)

target_link_libraries(Downloader
	PRIVATE
		${WIN32LIBS}
		${PRD_JSONCPP_LIBRARIES}
		pr-md5
		pr-sha1
		${archiveslib}
		${ZLIB_LIBRARIES}
	PUBLIC
		${CURL_LINK_LIBRARIES}
		${OPENSSL_LINK_LIBRARIES}
)

if(PRD_ARCHIVE_SUPPORT)
	if(MINIZIP_FOUND)
		target_link_libraries(Downloader PRIVATE ${MINIZIP_LIBRARIES} ${LZMA_LIBRARY})
	else()
		target_link_libraries(Downloader PRIVATE pr-minizip)
	endif()
endif()


set(PRDOWNLOADER "pr-downloader")
set(PRDOWNLOADER_SHARED ${PRDOWNLOADER}_shared)
set(PRDOWNLOADER_STATIC ${PRDOWNLOADER}_static)

option(PRD_SHAREDLIB "Enables compilation of the shared lib" ON)

if(PRD_SHAREDLIB)
	add_library(${PRDOWNLOADER_SHARED} SHARED
		pr-downloader.cpp
		Logger.cpp)
	if(PRD_DO_INSTALL AND (PRD_DEVELOP_FILES OR (PRD_CONSOLETOOL AND NOT PRD_LINK_STATIC)))
		install(TARGETS ${PRDOWNLOADER_SHARED}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR})
	endif()
	target_include_directories(${PRDOWNLOADER_SHARED} PRIVATE ${pr-downloader_SOURCE_DIR}/src)
	target_link_libraries( ${PRDOWNLOADER_SHARED} ${PRDOWNLOADER_LIBS} )
endif()

if(PRD_DEFAULT_LOGGER)
	set(LOGGERCPP Logger.cpp)
else()
	set(LOGGERCPP)
endif()

option(PRD_STATICLIB "Enables compilation of the static lib" ON)
if(PRD_STATICLIB)
	add_library(${PRDOWNLOADER_STATIC} STATIC
		pr-downloader.cpp
		${LOGGERCPP})
	if(PRD_DO_INSTALL AND PRD_DEVELOP_FILES)
		install(TARGETS ${PRDOWNLOADER_STATIC}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR})
	endif()
	target_include_directories(${PRDOWNLOADER_STATIC}
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}/lib/7z
	)
	target_link_libraries(${PRDOWNLOADER_STATIC} ${PRDOWNLOADER_LIBS} ${CMAKE_DL_LIBS})
endif()

option(PRD_CONSOLETOOL "Enables compilation and installation of the console tool ${PRDOWNLOADER}" ON)

if(PRD_CONSOLETOOL)
	### Add icon and manifest to exe using windres
	if(WIN32 AND MINGW)
	        set(PRD_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
	endif()

	add_executable(${PRDOWNLOADER}
		${PRD_ICON}
		main.cpp
		Logger.cpp)
	target_include_directories(${PRDOWNLOADER} PRIVATE ${pr-downloader_SOURCE_DIR}/src)

	if(PRD_LINK_STATIC)
		target_link_libraries( ${PRDOWNLOADER} ${PRDOWNLOADER_STATIC} )
	else()
		# hack arround "make install" doesn't generate libpr-downloader_shared.so
		add_dependencies(${PRDOWNLOADER} ${PRDOWNLOADER_SHARED})
		target_link_libraries(${PRDOWNLOADER} ${PRDOWNLOADER_SHARED})
	endif()
	if(WIN32)
		set_target_properties(${PRDOWNLOADER} PROPERTIES LINK_FLAGS "-Wl,-subsystem,console")
	endif()
	if(PRD_DO_INSTALL)
		install(TARGETS ${PRDOWNLOADER}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR})
	endif()
endif()

file(GLOB_RECURSE header "*.h")

if(NOT WIN32 AND PRD_DEVELOP_FILES)
	set(VERSION ${PR_DOWNLOADER_VERSION})
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libspringdownloader.pc.in"
	               "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" @ONLY)
	if(PRD_DO_INSTALL)
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" DESTINATION "${PRD_LIBDIR}/pkgconfig")
	endif()
endif()

if(PRD_DO_INSTALL AND PRD_DEVELOP_FILES)
	install(FILES
		pr-downloader.h
	        DESTINATION ${PRD_INCLUDE_DIR}/Downloader COMPONENT Devel)
endif()

option(PRD_ENABLE_LSL "compile lsl modules" OFF)
if(PRD_ENABLE_LSL)
	add_subdirectory(lsl)
endif()

option(RAPIDTOOLS "compile rapid tools" OFF)
if(RAPIDTOOLS)
	pkg_check_modules(LIBGIT2 libgit2)
	pkg_check_modules(LUA REQUIRED lua51)
	pkg_check_modules(ZIP REQUIRED libzip)

	add_library(Rapid
		${SRC_OS}
		rapid/ArchiveEntry.cpp
		rapid/BitArray.cpp
		rapid/Crc32.cpp
		rapid/FileEntry.cpp
		rapid/Gzip.cpp
		rapid/Hex.cpp
		rapid/Last.cpp
		rapid/LastGit.cpp
		rapid/Lua.cpp
		rapid/Marshal.cpp
		rapid/Md5.cpp
		rapid/Optional.cpp
		rapid/PoolArchive.cpp
		rapid/PoolFile.cpp
		rapid/ScopeGuard.cpp
		rapid/Store.cpp
		rapid/String.cpp
		rapid/TempFile.cpp
		rapid/Versions.cpp
		rapid/Zip.cpp
		rapid/ZipFile.cpp
		Logger.cpp)

	target_link_libraries(Rapid
		pr-md5
		${LUA_LIBRARIES}
		${ZIP_LIBRARIES}
		${ZLIB_LIBRARIES})
	if(Libgit2_FOUND)
		target_link_libraries(Rapid ${LIBGIT2_LIBRARIES})
		target_include_directories(Rapid PRIVATE ${LIBGIT2_HEADERS})
	else()
		target_link_libraries(Rapid git2)
		target_include_directories(Rapid
			PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/libgit2/include
			PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libgit2/src
			PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libgit2/deps/http-parser)
	endif()

	target_include_directories(Rapid
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
		PUBLIC ${LUA_INCLUDE_DIR}
		PUBLIC ${ZIP_INCLUDE_DIR}
		PUBLIC ${ZLIB_INCLUDE_DIR})
	if(MINIZIP_FOUND)
		target_link_libraries(Rapid ${MINIZIP_LIBRARIES})
	else()
		target_link_libraries(Rapid pr-minizip)
	endif()

	add_executable(AddZip AddZip.cpp)
	target_link_libraries(AddZip Rapid)

	add_executable(BuildGit BuildGit.cpp)
	target_link_libraries(BuildGit Rapid)

	add_executable(MakeZip MakeZip.cpp)
	target_link_libraries(MakeZip Rapid)

	add_executable(Streamer Streamer.cpp)
	target_link_libraries(Streamer Rapid)
endif()

