# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER /opt/mingw32/bin/i586-pc-mingw32-gcc)
SET(CMAKE_CXX_COMPILER /opt/mingw32/bin/i586-pc-mingw32-g++)
# here is the target environment located

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
# SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET( CMAKE_VERBOSE_MAKEFILE OFF )
#otherwise cmake finds linux lib for win...
SET( Boost_LIBRARIES
	boost_thread-gcc44-mt-1_41
	boost_filesystem-gcc44-mt-1_41
	boost_system-gcc44-mt-1_41 )
SET( boost_LIB_DIR /opt/mingw32/lib )
SET( boost_INCLUDE_DIR /opt/mingw32/include )
SET( WINDRES /opt/mingw32/bin/i586-pc-mingw32-windres )
link_directories( /opt/mingw32/lib )
INCLUDE_DIRECTORIES(/opt/mingw32/include  )
INCLUDE_DIRECTORIES(/opt/mingw32/include/drmingw/include  )

SET( PKG_CONFIG_EXECUTABLE /opt/mingw32/bin/pkg-config )
ADD_DEFINITIONS( -mthreads)

SET( CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE STRING
	"install prefix" FORCE )
