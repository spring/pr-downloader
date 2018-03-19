#include "PoolFile.h"

namespace Rapid {

PoolFileT::PoolFileT(StoreT & Store)
:
	mStore(Store),
	mTempFile{Store},
	mSize{0}
{}

void PoolFileT::write(void const * Buffer, unsigned Length)
{
	mTempFile.getOut().write(Buffer, Length);
	mMd5.update(Buffer, Length);
	mCrc.update(Buffer, Length);
	mSize += Length;
}

FileEntryT PoolFileT::close()
{
	FileEntryT Entry;
	Entry.Digest = mMd5.final();
	Entry.Checksum = mCrc.final();
	Entry.Size = mSize;

	mTempFile.commit(mStore.getPoolPath(Entry.Digest));
	return Entry;
}

}
