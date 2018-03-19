#pragma once

#include <cstdlib>

#include "lib/md5/md5.h"

namespace Rapid {

struct DigestT
{
	unsigned char Buffer[16];
};

class Md5T
{
	private:
	MD5_CTX mContext;

	public:
	Md5T();
	void update(void const * Buffer, std::size_t Length);
	DigestT final();
};

}
