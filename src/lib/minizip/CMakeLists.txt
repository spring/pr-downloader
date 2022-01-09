# Spring supplied CMake build file

SET(miniZipSources
	unzip.c
	zip.c
	ioapi.c
)

FIND_PACKAGE(ZLIB REQUIRED)

if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
endif ()
add_definitions(-DNOCRYPT -DNOUNCRYPT)

ADD_LIBRARY(pr-minizip STATIC EXCLUDE_FROM_ALL ${miniZipSources})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(pr-minizip ${ZLIB_LIBRARY})
SET_TARGET_PROPERTIES(pr-minizip PROPERTIES COMPILE_FLAGS "${PIC_FLAG}")

