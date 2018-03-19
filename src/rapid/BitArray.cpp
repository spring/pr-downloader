#include "BitArray.h"

namespace Rapid {

void BitArrayT::append(char const * Bytes, std::size_t Size)
{
	mBytes.append(Bytes, Size);
}

std::size_t BitArrayT::size() const
{
	return mBytes.size() * 8;
}

bool BitArrayT::operator[](std::size_t Index) const
{
	auto ByteIndex = Index / 8;
	auto BitIndex = Index % 8;
	return static_cast<std::uint8_t>(mBytes[ByteIndex]) >> BitIndex & 0x1;
}

}
