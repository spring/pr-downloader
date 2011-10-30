

FIND_PACKAGE(Boost 1.35.0 COMPONENTS unit_test_framework)
IF	(NOT Boost_FOUND)
	Message(STATUS "Note: Unit tests will not be built: Boost::test library was not found")
Else	(NOT Boost_FOUND)

EndIf (NOT Boost_FOUND)

