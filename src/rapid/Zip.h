#pragma once

#include "ZipFile.h"

#include <string>
#include <stdexcept>

#include <zip.h>

namespace Rapid {

class ZipT
{
	private:
	zip * mZip;

	public:
	ZipT(std::string const & Path, int Flags);
	~ZipT();

	template<typename FunctorT>
	void iterateFiles(FunctorT Functor)
	{
		struct zip_stat Stat;
		auto NumEntries = zip_get_num_files(mZip);
		for (int I = 0; I != NumEntries; ++I)
		{
			auto Error = zip_stat_index(mZip, I, 0, &Stat);
			if (Error != 0) throw std::runtime_error{"Error reading zip"};
			auto Length = std::char_traits<char>::length(Stat.name);
			if (Length == 0 || Stat.name[Length - 1] == '/') continue;
			auto File = ZipFileT{Stat.name, zip_fopen_index(mZip, I, 0)};
			Functor(File);
		}
	}
};

}
