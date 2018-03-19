#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <zlib.h>

namespace Rapid {

class GzipT
{
	private:
	gzFile mFile;
	std::string mPath;

	public:
	GzipT();
	GzipT(std::string const & Path, char const * Mode);
	~GzipT();

	void open(std::string const & Path, char const * Mode);
	void close();
	unsigned read(void * Buffer, unsigned Length);
	bool readMaybe(void * Buffer, unsigned Length);
	void readExpected(void * Buffer, unsigned Length);
	void write(void const * Buffer, unsigned Length);
	void write(char Char);

	static std::string readFile(std::string const & Path);
};

}
