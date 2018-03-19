# -*- cmake -*-

# - Find Apache Portable Runtime
# Find the ZIP includes and libraries
# This module defines
#  ZIP_INCLUDE_DIR and ZIPUTIL_INCLUDE_DIR, where to find zip.h, etc.
#  ZIP_LIBRARIES and ZIPUTIL_LIBRARIES, the libraries needed to use ZIP.
#  ZIP_FOUND and ZIPUTIL_FOUND, If false, do not try to use ZIP.
# also defined, but not for general use are
#  ZIP_LIBRARY where to find the ZIP library.


find_path (ZIP_INCLUDE_DIR zip.h
	/usr/local/include)

set (ZIP_NAMES ${ZIP_NAMES} zip)
find_library (ZIP_LIBRARY
	NAMES ${ZIP_NAMES}
	PATHS /usr/lib /usr/local/lib)

if (ZIP_LIBRARY AND ZIP_INCLUDE_DIR)
	set (ZIP_LIBRARIES ${ZIP_LIBRARY})
	set (ZIP_FOUND "YES")
else ()
	set (ZIP_FOUND "NO")
endif ()

if (ZIP_FOUND)
	if (NOT ZIP_FIND_QUIETLY)
		message (STATUS "Found ZIP: ${ZIP_LIBRARIES}")
	endif ()
else (ZIP_FOUND)
	if (ZIP_FIND_REQUIRED)
		message (FATAL_ERROR "Could not find ZIP library")
	endif ()
endif ()

mark_as_advanced (
	ZIP_LIBRARY
	ZIP_INCLUDE_DIR)
