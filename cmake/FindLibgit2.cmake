# - Try to find the libgit2 library
# Once done this will define
#
#  LIBGIT2_FOUND - System has libgit2
#  LIBGIT2_INCLUDE_DIR - The libgit2 include directory
#  LIBGIT2_LIBRARIES - The libraries needed to use libgit2
#  LIBGIT2_DEFINITIONS - Compiler switches required for using libgit2


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
#FIND_PACKAGE(PkgConfig)
#PKG_SEARCH_MODULE(PC_LIBGIT2 libgit2)

set(LIBGIT2_DEFINITIONS ${PC_LIBGIT2_CFLAGS_OTHER})

find_path(LIBGIT2_INCLUDE_DIR NAMES git2.h
	HINTS
	${PC_LIBGIT2_INCLUDEDIR}
	${PC_LIBGIT2_INCLUDE_DIRS}
)

if (LIBGIT2_INCLUDE_DIR AND EXISTS "${LIBGIT2_INCLUDE_DIR}/git2/version.h")
	file(STRINGS "${LIBGIT2_INCLUDE_DIR}/git2/version.h" LIBGIT2_H REGEX "^#define LIBGIT2_VERSION \"[^\"]*\"$")

	string(REGEX REPLACE "^.*LIBGIT2_VERSION \"([0-9]+).*$" "\\1" LIBGIT2_VERSION_MAJOR "${LIBGIT2_H}")
	string(REGEX REPLACE "^.*LIBGIT2_VERSION \"[0-9]+\\.([0-9]+).*$" "\\1" LIBGIT2_VERSION_MINOR  "${LIBGIT2_H}")
	string(REGEX REPLACE "^.*LIBGIT2_VERSION \"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" LIBGIT2_VERSION_PATCH "${LIBGIT2_H}")
	set(LIBGIT2_VERSION_STRING "${LIBGIT2_VERSION_MAJOR}.${LIBGIT2_VERSION_MINOR}.${LIBGIT2_VERSION_PATCH}")
endif ()


find_library(LIBGIT2_LIBRARIES NAMES git2
	HINTS
	${PC_LIBGIT2_LIBDIR}
	${PC_LIBGIT2_LIBRARY_DIRS}
)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libgit2 REQUIRED_VARS LIBGIT2_LIBRARIES LIBGIT2_INCLUDE_DIR VERSION_VAR LIBGIT2_VERSION_STRING)

mark_as_advanced(LIBGIT2_INCLUDE_DIR LIBGIT2_LIBRARIES)

