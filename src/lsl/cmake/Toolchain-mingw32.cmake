# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/bin/i586-mingw32msvc /var/lib/buildbot/lib/mingw )

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
# SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET( CMAKE_VERBOSE_MAKEFILE ON )

SET( Boost_LIBRARIES libboost_thread-gcc42-mt-d-1_41.a boost_filesystem-gcc42-mt-d-1_41 boost_date_time-gcc42-mt-d-1_41 boost_system-gcc42-mt-d-1_41 )
SET( boost_LIB_DIR /var/lib/buildbot/lib/mingw/lib )
SET( boost_INCLUDE_DIR /var/lib/buildbot/lib/mingw/include )
link_directories( /var/lib/buildbot/lib/mingw/lib )
INCLUDE_DIRECTORIES(/var/lib/buildbot/lib/mingw/include  )


