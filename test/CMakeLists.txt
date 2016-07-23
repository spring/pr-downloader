

FIND_PACKAGE(Boost 1.35.0 COMPONENTS unit_test_framework)
IF(NOT Boost_FOUND)
	Message(STATUS "Note: Unit tests will not be built: Boost::test library was not found")
Else()
	If (NOT (WIN32 OR Boost_USE_STATIC_LIBS))
		#Win32 tests links static
		add_definitions(-DBOOST_TEST_DYN_LINK)
	EndIf()
	include_directories(../src)

	SET(prd_testsrc
		${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
	)

	ADD_EXECUTABLE(prd_test WIN32 MACOSX_BUNDLE ${prd_testsrc} )
	add_test(NAME prdtest COMMAND prd_test)

	TARGET_LINK_LIBRARIES(prd_test
		${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
		Downloader
		${CMAKE_DL_LIBS}
	)

	target_include_directories(prd_test
		PRIVATE ${Boost_INCLUDE_DIRS}
	)	
#	target_include_directories(libSpringLobby_test
#		PRIVATE ${libSpringLobby_SOURCE_DIR}/src
#	)

EndIf()

