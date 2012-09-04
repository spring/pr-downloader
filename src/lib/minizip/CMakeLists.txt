# Spring supplied CMake build file

SET(miniZipSources
	unzip.c
	zip.c
	ioapi.c
)
IF    (WIN32)
	LIST(APPEND miniZipSources iowin32.c)
ENDIF (WIN32)

FIND_PACKAGE(ZLIB REQUIRED)

ADD_LIBRARY(pr-minizip STATIC EXCLUDE_FROM_ALL ${miniZipSources})
TARGET_LINK_LIBRARIES(pr-minizip ${ZLIB_LIBRARY})
SET_TARGET_PROPERTIES(pr-minizip PROPERTIES COMPILE_FLAGS "${PIC_FLAG}")

