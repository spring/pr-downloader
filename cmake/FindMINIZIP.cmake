# This file is part of the Spring engine (GPL v2 or later), see LICENSE.html

# - Find the MiniZip library (sub-part of zlib)
# Find the native MiniZip includes and library (static or shared)
#
#  MINIZIP_INCLUDE_DIR - where to find zip.h, unzip.h, ioapi.h, etc.
#  MINIZIP_LIBRARIES   - List of libraries when using minizip.
#  MINIZIP_FOUND       - True if minizip was found.

include(FindPackageHandleStandardArgs)

if(MINIZIP_INCLUDE_DIR)
	# Already in cache, be silent
	set(MINIZIP_FIND_QUIETLY TRUE)
endif()

find_path(MINIZIP_INCLUDE_DIR minizip/zip.h)

set(MINIZIP_NAMES minizip)
find_library(MINIZIP_LIBRARY NAMES ${MINIZIP_NAMES})

# handle the QUIETLY and REQUIRED arguments and set MINIZIP_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(MINIZIP DEFAULT_MSG MINIZIP_LIBRARY MINIZIP_INCLUDE_DIR)

if(MINIZIP_FOUND)
	set(MINIZIP_LIBRARIES ${MINIZIP_LIBRARY})
else()
	set(MINIZIP_LIBRARIES)
endif()

mark_as_advanced(MINIZIP_LIBRARY MINIZIP_INCLUDE_DIR)
