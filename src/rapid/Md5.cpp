#include "Md5.h"
#include "string.h"
#include "lib/md5/md5.h"

namespace Rapid {

Md5T::Md5T()
{
	MD5Init(&mContext);
}

void Md5T::update(void const * Buffer, std::size_t Length)
{
	MD5Update(&mContext, (unsigned char*)Buffer, Length);
}

DigestT Md5T::final()
{
	DigestT Digest;
	MD5Final(&mContext);
	memcpy(Digest.Buffer, mContext.digest, 16);
	return Digest;
}

}

