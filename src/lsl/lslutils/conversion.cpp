/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "conversion.h"
#include <cassert>
#include <sstream>
#include <algorithm>

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#ifndef INT_MIN
#define INT_MIN (-2147483647 - 1)
#endif

namespace LSL
{
namespace Util
{

int64_t FromLongString(const std::string& s)
{
	int64_t n = 0;
	int64_t sign = 1;
	const char* str = s.c_str();

	if (*str == '-') {
		sign = -1;
		++str;
	} else if (*str == '+') {
		sign = 1;
		++str;
	}
	while (isdigit(*str)) {
		n *= 10;
		int ch = *str - '0';
		n += ch;
		++str;
	}
	return sign * n;
}

int32_t FromIntString(const std::string& s)
{
	int n = 0;
	int sign = 1;
	const char* str = s.c_str();

	if (*str == '-') {
		sign = -1;
		++str;
	} else if (*str == '+') {
		sign = 1;
		++str;
	}
	while (isdigit(*str)) {
		n *= 10;
		int ch = *str - '0';
		n += ch;
		++str;
	}
	return sign * n;
}

float FromFloatString(const std::string& s)
{
	std::stringstream ss;
	ss.imbue(std::locale("C"));
	for(size_t i = 0; i < s.size(); i++) {
		if (s[i] == ',')
			ss << '.';
		else
			ss << s[i];
	}
	float ret = 0.0f;
	ss >> ret;
	return ret;
}

std::string ToIntString(int i)
{
	char output[32];
	const int num = snprintf(output, 32, "%d", i);
	return std::string(output, num);
}

std::string ToUIntString(int i)
{
	char output[32];
	const int num = snprintf(output, 32, "%u", i);
	return std::string(output, num);
}


std::string ToFloatString(float f)
{
	std::stringstream str;
	str << f;
	return str.str();
}

std::string MakeHashUnsigned(const std::string& hash)
{
	return ToUIntString(FromIntString(hash));
}

std::string MakeHashSigned(const std::string& hash)
{
	return ToIntString(FromIntString(hash));
}

// convert const char* to std::string, as std::string(NULL) crashes
std::string SafeString(const char* str)
{
	if (str == NULL)
		return "";
	return std::string(str);
}

std::string ToLower(const std::string& s)
{
	std::string res = s;
	std::transform(res.begin(), res.end(), res.begin(), ::tolower);
	return res;
}


#ifdef WIN32
#include <windows.h>
#include <string>

std::wstring s2ws(const std::string& s)
{
	const size_t slength = s.length();
	const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf, len);
	delete[] buf;
	return r;
}

std::string ws2s(const std::wstring& s)
{
	const size_t slength = s.length();
	const int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, NULL, 0, NULL, NULL);
	char* buf = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, buf, len, NULL, NULL);
	std::string r(buf, len);
	delete[] buf;
	return r;
}


std::string geterrormsg()
{
	const int code = GetLastError();
	static const int bufsize = 256;
	char lpBuffer[bufsize];
	const int len = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpBuffer, bufsize - 1, NULL);
	return std::string(lpBuffer, len);
}

#endif //WIN32

} // namespace Util
} // namespace LSL
