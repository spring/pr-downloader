/* This file is part of the Springlobby (GPL v2 or later), see COPYING */
#ifndef LSL_HEADERGUARD_DEBUG_H
#define LSL_HEADERGUARD_DEBUG_H

#include <stdexcept>
#include <string>
#include "logging.h"

namespace LSL
{
namespace Exceptions
{

struct base : public std::runtime_error {
	base(const std::string& msg)
	    : std::runtime_error(msg)
	{
	}
};
struct file_not_writable : public base {
	file_not_writable(const std::string& msg)
	    : base(msg)
	{
	}
};
struct file_not_found : public base {
	file_not_found(const std::string& msg)
	    : base(msg)
	{
	}
};
struct unitsync : public base {
	unitsync(const std::string& msg)
	    : base("UNITSYNC: " + msg)
	{
	}
};
struct conversion : public base {
	conversion(const std::string& msg)
	    : base("conversion failed: " + msg)
	{
	}
};
struct battle : public base {
	battle(const std::string& msg)
	    : base("battle exception: " + msg)
	{
	}
};
struct server : public base {
	server(const std::string& msg)
	    : base("server exception: " + msg)
	{
	}
};
struct function_missing : public unitsync {
	function_missing(const std::string& funcname)
	    : unitsync(" function couldn't be imported: " + funcname)
	{
	}
};
} // namespace Exceptions
} // namespace LSL


#define LSL_THROW(excp, msg)                              \
	do {                                              \
		LslDebug("%s -- %d", __FILE__, __LINE__); \
		throw LSL::Exceptions::excp(msg);         \
	} while (0)
#define LSL_THROWF(excp, msg, ...)                                                        \
	do {                                                                              \
		char buf[1024];                                                           \
		snprintf(buf, 1024, "%s -- %d: " msg, __FILE__, __LINE__, ##__VA_ARGS__); \
		LslDebug("%s", buf);                                                      \
		throw LSL::Exceptions::excp(buf);                                         \
	} while (0)


#endif // LSL_HEADERGUARD_DEBUG_H
