#pragma once

#include "Gzip.h"
#include "Crc32.h"
#include "Md5.h"
#include "FileEntry.h"
#include "Store.h"
#include "TempFile.h"

#include <string>

namespace Rapid {

class PoolFileT
{
	private:
	StoreT & mStore;
	TempFileT mTempFile;
	Md5T mMd5;
	Crc32T mCrc;
	std::uint32_t mSize;

	public:
	PoolFileT(StoreT & Store);
	void write(void const * Buffer, unsigned Length);
	FileEntryT close();
};

}
