#ifndef LSL_TESTS_COMMON_H
#define LSL_TESTS_COMMON_H

#include <exception>
#include <string>

struct TestFailedException : public std::logic_error
{
	TestFailedException(std::string msg)
	    : std::logic_error(msg)
	{
	}
};

#endif // LSL_TESTS_COMMON_H
