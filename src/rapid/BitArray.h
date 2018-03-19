#pragma once

#include <string>

namespace Rapid {

class BitArrayT
{
	private:
	std::string mBytes;

	public:
	void append(char const * Bytes, std::size_t Size);
	std::size_t size() const;
	bool operator[](std::size_t Index) const;
};

}
