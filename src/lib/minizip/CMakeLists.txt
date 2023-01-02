# Spring supplied CMake build file

set(miniZipSources
	unzip.c
	zip.c
	ioapi.c
)

find_package(ZLIB REQUIRED)

if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
endif()
add_definitions(-DNOCRYPT -DNOUNCRYPT)

add_library(pr-minizip STATIC EXCLUDE_FROM_ALL ${miniZipSources})
target_include_directories(pr-minizip
	PUBLIC
		${ZLIB_INCLUDE_DIR}
)
target_link_libraries(pr-minizip ${ZLIB_LIBRARY})
set_target_properties(pr-minizip PROPERTIES COMPILE_FLAGS "${PIC_FLAG}")

