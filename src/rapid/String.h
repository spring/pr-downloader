#pragma once

#include <array>
#include <cstdlib>
#include <cstring>
#include <string>

namespace Rapid {

inline
std::size_t getLength(std::string const & String)
{
	return String.size();
}

inline
std::size_t getLength(char const * String)
{
	return std::strlen(String);
}

inline
std::size_t getLength(char)
{
	return 1;
}

template<std::size_t Size>
std::size_t getLength(std::array<char, Size>)
{
	return Size;
}

template<typename ... ArgsT>
std::size_t getLengthHelper(std::size_t Accu)
{
	return Accu;
}

template<typename T, typename ... ArgsT>
std::size_t getLengthHelper(std::size_t Accu, T const & Object, ArgsT const & ... Args)
{
	return getLengthHelper(Accu + getLength(Object), Args ...);
}

template<typename ... ArgsT>
std::size_t getLength(ArgsT const & ... Args)
{
	return getLengthHelper(0, Args ...);
}

inline
void append(std::string & String, char const * Data)
{
	String.append(Data);
}

inline
void append(std::string & String, std::string const & Data)
{
	String.append(Data);
}

inline
void append(std::string & String, char Data)
{
	String.push_back(Data);
}

template<std::size_t Size>
void append(std::string & String, std::array<char, Size> const & Data)
{
	String.append(Data.begin(), Data.end());
}

template<typename ... ArgsT>
void appendHelper(std::string &)
{}

template<typename T, typename ... ArgsT>
void appendHelper(std::string & String, T const & Object, ArgsT const & ... Args)
{
	append(String, Object);
	appendHelper(String, Args ...);
}

template<typename ... ArgsT>
std::string & concatAppend(std::string & String, ArgsT const & ... Args)
{
	appendHelper(String, Args ...);
	return String;
}

template<typename ... ArgsT>
std::string & concatReplace(std::string & String, ArgsT const & ... Args)
{
	String.clear();
	appendHelper(String, Args ...);
	return String;
}

template<typename ... ArgsT>
std::string & concatAt(std::string & String, std::size_t Index, ArgsT const & ... Args)
{
	String.resize(Index);
	appendHelper(String, Args ...);
	return String;
}

template<typename ... ArgsT>
std::string concat(ArgsT const & ... Args)
{
	std::string String;
	auto Length = getLength(Args ...);
	String.reserve(Length);
	appendHelper(String, Args ...);
	return String;
}

}
