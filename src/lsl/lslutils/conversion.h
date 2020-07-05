/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_CONVERSION_H
#define LSL_CONVERSION_H

#include <string>
#include <cstring>

namespace LSL
{
namespace Util
{

int64_t FromLongString(const std::string& s);
int32_t FromIntString(const std::string& s);
float FromFloatString(const std::string& s);
std::string ToIntString(int i);
std::string ToUIntString(int i);
std::string ToFloatString(float f);
std::string ToLower(const std::string& s);

std::string MakeHashUnsigned(const std::string& hash);
std::string MakeHashSigned(const std::string& hash);

// convert const char* to std::string, as std::string(NULL) crashes
std::string SafeString(const char* str);

#ifdef _WIN32
// convert std::string to std::wstring
std::wstring s2ws(const std::string& s);
std::string ws2s(const std::wstring& s);
std::string geterrormsg();
#endif

} // namespace Util
} // namespace LSL

#endif // LSL_CONVERSION_H
