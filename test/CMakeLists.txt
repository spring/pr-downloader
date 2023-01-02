

find_package(Boost 1.35.0 COMPONENTS unit_test_framework)
if(NOT Boost_FOUND)
	message(STATUS "Note: Unit tests will not be built: Boost::test library was not found")
else()
	add_custom_target(checkDownloader ${CMAKE_CTEST_COMMAND} --output-on-failure)
	if(NOT (WIN32 OR Boost_USE_STATIC_LIBS))
		#Win32 tests links static
		add_definitions(-DBOOST_TEST_DYN_LINK)
	endif()

	set(prd_testsrc
		test.cpp
		../src/Logger.cpp)

	add_executable(prd_test WIN32 MACOSX_BUNDLE ${prd_testsrc})
	add_test(NAME prdtest COMMAND prd_test)

	target_link_libraries(prd_test
		${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
		Downloader
		${CMAKE_DL_LIBS})

	target_include_directories(prd_test
		PRIVATE
			${Boost_INCLUDE_DIRS}
			${pr-downloader_SOURCE_DIR}/src)


################################################################################
### libSpringLobby

if(PRD_ENABLE_LSL)

set(basic_testSrc
	${CMAKE_CURRENT_SOURCE_DIR}/lsl/basic.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lsl/usync.cpp
	${pr-downloader_SOURCE_DIR}/src/Logger.cpp
)

add_executable(libSpringLobby_test WIN32 MACOSX_BUNDLE ${basic_testSrc})
add_test(NAME libSpringLobbyTest COMMAND libSpringLobby_test)


target_link_libraries(libSpringLobby_test dl lsl-unitsync dl Downloader)
target_include_directories(libSpringLobby_test
	PRIVATE
		${pr-downloader_SOURCE_DIR}/src/lsl
)
if(NOT WIN32)
	target_link_libraries(libSpringLobby_test X11)
endif()

endif()

################################################################################
### swig

add_executable(swig_test WIN32 MACOSX_BUNDLE ${CMAKE_CURRENT_SOURCE_DIR}/lsl/swig.cpp)
add_test(NAME swigTest COMMAND swig_test)

endif()

