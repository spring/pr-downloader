#include "rapid/PoolArchive.h"
#include "rapid/Versions.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>

namespace {

using namespace Rapid;

void makeZip(std::string StorePath, std::string Tag, std::string Output)
{
	StoreT Store{StorePath};
	PoolArchiveT Archive{Store};
	VersionsT Versions{Store};
	Versions.load();
	auto Entry = Versions.findTag(Tag);
	Archive.load(Entry.Digest);
	Archive.makeZip(Output);
}

}

int main(int argc, char const * const * argv, char const * const * env)
{
	umask(0002);

	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " <Store Path> <Rapid Tag> <Output>\n";
		return 1;
	}

	try
	{
		makeZip(argv[1], argv[2], argv[3]);
	}
	catch (std::exception const & Exception)
	{
		std::cerr << Exception.what() << "\n";
		return 1;
	}
}
