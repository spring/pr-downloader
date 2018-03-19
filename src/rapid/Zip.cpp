#include "Zip.h"

#include <stdexcept>

namespace Rapid {


ZipT::ZipT(std::string const & Path, int Flags)
{
	int Error;
	mZip = zip_open(Path.c_str(), Flags, &Error);
	if (mZip == nullptr) throw std::runtime_error{"Error opening zip"};
}

ZipT::~ZipT()
{
	zip_close(mZip);
}

}
