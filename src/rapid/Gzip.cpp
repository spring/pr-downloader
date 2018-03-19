#include "Gzip.h"

#include <stdexcept>

namespace Rapid {

GzipT::GzipT()
:
	mFile{nullptr}
{}

GzipT::GzipT(std::string const & Path, char const * Mode)
{
	mFile = gzopen(Path.c_str(), Mode);
	mPath = Path;
	if (mFile == nullptr) throw std::runtime_error{"Error opening gzip:" + mPath};
}

void GzipT::open(std::string const & Path, char const * Mode)
{
	if (mFile == nullptr) throw std::runtime_error{"Gzip already open:" + mPath};
	mFile = gzopen(Path.c_str(), Mode);
	mPath = Path;
	if (mFile == nullptr) throw std::runtime_error{"Error opening gzip:" + mPath};
}

GzipT::~GzipT()
{
	gzclose(mFile);
}

void GzipT::close()
{
	gzclose(mFile);
	mFile = nullptr;
}

unsigned GzipT::read(void * Buffer, unsigned Length)
{
	auto ReadBytes = gzread(mFile, static_cast<char *>(Buffer), Length);
	if (ReadBytes == -1) throw std::runtime_error{"Error reading gzip:" + mPath};
	return static_cast<unsigned>(ReadBytes);
}

void GzipT::readExpected(void * Buffer, unsigned Length)
{
	auto ReadBytes = read(Buffer, Length);
	if (ReadBytes != Length) throw std::runtime_error{"Error reading gzip:" + mPath};
}

// o/` Hey, I just met you, and this is crazy, but here's my buffer, so read me maybe o/`

bool GzipT::readMaybe(void * Buffer, unsigned Length)
{
	auto ReadBytes = read(Buffer, Length);
	if (ReadBytes != Length) return false;
	else return true;
}

void GzipT::write(void const * Buffer, unsigned Length)
{

	if (Length==0)
		return;
	auto Error = gzwrite(mFile, Buffer, Length);
	if (Error == 0) throw std::runtime_error{"Error writing gzip buffer:" + mPath};
}

void GzipT::write(char Char)
{
	auto Error = gzputc(mFile, Char);
	if (Error == 0) throw std::runtime_error{"Error writing gzip char" + mPath};
}

std::string GzipT::readFile(std::string const & Path)
{
	GzipT In{Path, "rb"};
	std::string Result;
	char Buffer[4096];

	while (true)
	{
		auto ReadBytes = In.read(Buffer, 4096);
		if (ReadBytes == 0) break;
		Result.append(Buffer, Buffer + ReadBytes);
	}

	return Result;
}

}
