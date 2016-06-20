#define BOOST_TEST_MODULE Float3
#include <boost/test/unit_test.hpp>

#include "FileSystem/FileSystem.h"

BOOST_AUTO_TEST_CASE( prd )
{
	BOOST_CHECK("_____" == CFileSystem::EscapeFilename("/<|>/"));
	BOOST_CHECK("_____" == CFileSystem::EscapeFilename("/<|>\\"));
	BOOST_CHECK("abC123" == CFileSystem::EscapeFilename("abC123"));
}
