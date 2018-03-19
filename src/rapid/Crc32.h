#pragma once

#include <cstdint>

namespace Rapid {

typedef std::uint32_t ChecksumT;

class Crc32T
{
	private:
	std::uint32_t mCrc;

	public:
	Crc32T();

	void update(void const * Buffer, std::size_t Length);
	ChecksumT final();
};

}
