#include "Hex.h"

namespace Rapid {
namespace Hex {

void decode(char const * Source, unsigned char * Dest, std::size_t Length)
{
	for (std::size_t Index = 0; Index != Length; ++Index)
	{
		auto High = Source[2 * Index];
		auto Low = Source[2 * Index + 1];
		Dest[Index] = (DecodeTable[High - 48] << 4) | DecodeTable[Low - 48];
	}
}

void encode(char * Dest, unsigned char const * Source, std::size_t Length)
{
	for (std::size_t Index = 0; Index != Length; ++Index)
	{
		auto Byte = Source[Index];
		Dest[Index * 2] = EncodeTable[(Byte >> 4) & 0xf];
		Dest[Index * 2 + 1] = EncodeTable[Byte & 0xf];
	}
}

}
}
