#pragma once

#include <stdexcept>

#include <zip.h>

namespace Rapid {

class ZipFileT
{
	private:
	std::string mName;
	zip_file * mFile;

	public:
	ZipFileT(std::string Name, zip_file * File);
	~ZipFileT();

	std::string const & getName() const;

	template<typename FunctorT>
	void cat(FunctorT Functor) const
	{
		char Buffer[4096];
		while (true)
		{
			auto Bytes = zip_fread(mFile, Buffer, 4096);
			if (Bytes == 0) break;
			if (Bytes == -1) throw std::runtime_error{"Error reading zip"};
			Functor(Buffer, Bytes);
		}
	}
};

}
