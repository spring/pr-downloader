

FIND_PACKAGE(Boost 1.35.0 COMPONENTS unit_test_framework)
IF(NOT Boost_FOUND)
	Message(STATUS "Note: Unit tests will not be built: Boost::test library was not found")
Else()
	add_custom_target(checkDownloader ${CMAKE_CTEST_COMMAND} --output-on-failure)
	If (NOT (WIN32 OR Boost_USE_STATIC_LIBS))
		#Win32 tests links static
		add_definitions(-DBOOST_TEST_DYN_LINK)
	EndIf()
	include_directories(../src)

	SET(prd_testsrc
		${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
		${pr-downloader_SOURCE_DIR}/src/Logger.cpp
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


################################################################################
### libSpringLobby

if (PRD_ENABLE_LSL)

SET(basic_testSrc
	${CMAKE_CURRENT_SOURCE_DIR}/lsl/basic.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lsl/usync.cpp
	${pr-downloader_SOURCE_DIR}/src/Logger.cpp
	)

ADD_EXECUTABLE(libSpringLobby_test WIN32 MACOSX_BUNDLE ${basic_testSrc} )
add_test(NAME libSpringLobbyTest COMMAND libSpringLobby_test)


TARGET_LINK_LIBRARIES(libSpringLobby_test dl lsl-unitsync dl)
target_include_directories(libSpringLobby_test
		PRIVATE ${pr-downloader_SOURCE_DIR}/src/lsl
	)
IF( NOT WIN32 )
	TARGET_LINK_LIBRARIES(libSpringLobby_test X11 )
ENDIF()

endif(PRD_ENABLE_LSL)

################################################################################
### swig

ADD_EXECUTABLE(swig_test WIN32 MACOSX_BUNDLE ${CMAKE_CURRENT_SOURCE_DIR}/lsl/swig.cpp )
add_test(NAME swigTest COMMAND swig_test)



EndIf()

