add_custom_target(check ${CMAKE_CTEST_COMMAND} --output-on-failure)

################################################################################
### libSpringLobby

SET(basic_testSrc
	${CMAKE_CURRENT_SOURCE_DIR}/basic.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/usync.cpp
	)

ADD_EXECUTABLE(libSpringLobby_test WIN32 MACOSX_BUNDLE ${basic_testSrc} )
add_test(NAME libSpringLobbyTest COMMAND libSpringLobby_test)


TARGET_LINK_LIBRARIES(libSpringLobby_test dl lsl-unitsync dl)
target_include_directories(libSpringLobby_test
		PRIVATE ${libSpringLobby_SOURCE_DIR}/src
	)
IF( NOT WIN32 )
	TARGET_LINK_LIBRARIES(libSpringLobby_test X11 )
ENDIF()

################################################################################
### swig

ADD_EXECUTABLE(swig_test WIN32 MACOSX_BUNDLE ${CMAKE_CURRENT_SOURCE_DIR}/swig.cpp )
add_test(NAME swigTest COMMAND swig_test)

