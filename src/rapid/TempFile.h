#pragma once

#include "Gzip.h"
#include "Store.h"

namespace Rapid {

class TempFileT
{
	private:
	std::string mPath;
	GzipT mOut;

	public:
	TempFileT(StoreT & Store);
	~TempFileT();

	GzipT & getOut();
	void commit(std::string const & Path);
};

}
