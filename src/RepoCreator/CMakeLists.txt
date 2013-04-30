cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(BUILD_SHARED_LIBS OFF)
set(BUILD_CLAR OFF)
add_subdirectory(libgit2)

#find_package(Libgit2 REQUIRED)
#include_directories(${LIBGIT2_INCLUDE_DIR})

include_directories(libgit2/include)
add_executable(	repocreator
		main.cpp
	)
#target_link_libraries(repocreator ${LIBGIT2_LIBRARIES})
target_link_libraries(repocreator git2)
