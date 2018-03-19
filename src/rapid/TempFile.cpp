#include "TempFile.h"

#include <stdexcept>

#include <unistd.h>

namespace Rapid {

TempFileT::TempFileT(StoreT & Store)
:
	mPath{Store.getTempPath()},
	mOut{mPath, "wb9"}
{}

TempFileT::~TempFileT()
{
	if (mPath.empty()) return;

	unlink(mPath.c_str());
}

GzipT & TempFileT::getOut()
{
	return mOut;
}

void TempFileT::commit(std::string const & Path)
{
	mOut.close();
	auto Error = rename(mPath.c_str(), Path.c_str());
	if (Error != 0) throw std::runtime_error{"Error renaming file" + mPath + " to " + Path};
	mPath.clear();

}

}
