#pragma once

#include "Crc32.h"
#include "Md5.h"

#include <cstdint>

namespace Rapid {

struct FileEntryT
{
	std::uint32_t Size;
	DigestT Digest;
	ChecksumT Checksum;
};

}
