#pragma once

#include <cstdint>

namespace Rapid {
namespace Marshal {

void packLittle(std::uint32_t Word, unsigned char * Bytes);
void unpackLittle(std::uint32_t & Word, unsigned char const * Bytes);
void packBig(std::uint32_t Word, unsigned char * Bytes);
void unpackBig(std::uint32_t & Word, unsigned char const * Bytes);
}
}
