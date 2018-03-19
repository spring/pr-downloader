#include "LastGit.h"

#include "Marshal.h"
#include "TempFile.h"

#include "Hex.h"

#include <sys/stat.h>
#include <sys/types.h>

namespace Rapid {

void LastGitT::save(LastGitT const & Last, StoreT & Store, std::string const & Prefix)
{
	TempFileT Temp{Store};
	Temp.getOut().write(Last.Hex.data(), 20);
	Temp.getOut().write(Last.Digest.Buffer, 16);
	Temp.commit(Store.getLastGitPath(Prefix));
}

OptionalT<LastGitT> LastGitT::load(StoreT & Store, std::string const & Prefix)
{


	auto Path = Store.getLastGitPath(Prefix);

	struct stat Stats;
	auto Error = stat(Path.c_str(), &Stats);
	if (Error == -1)
	{
		// If there was no last for this prefix/base, we use revision 0
		if (errno == ENOENT)
		{
			return {};
		}
		else throw std::runtime_error{"Error loading last"};
	}

	GzipT In{Path, "rb"};
	LastGitT Last;
	In.readExpected(Last.Hex.data(), 20);
	In.readExpected(Last.Digest.Buffer, 16);

	return {Last};
}

}
