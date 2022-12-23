/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "rapid/GitArchive.h"
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>

std::string fixRoot(std::string const & root)
{
	if ((root == ".") || (root == "/")) return "";
	else return root;
}

int main(int argc, char const * const * argv)
{
	umask(0002);

	if (argc != 7)
	{
		std::cerr << "Usage: " << argv[0] <<
			" <Git Path> <Mod Root> <Modinfo> <Store Path> <Git Hash> <Prefix>\n";
		return 1;
	}

	try
	{
		Rapid::buildGit(
			argv[1],
			fixRoot(argv[2]),
			argv[3],
			argv[4],
			argv[5],
			argv[6]);
	}
	catch (std::exception const & Exception)
	{
		std::cerr << Exception.what() << "\n";
		return 1;
	}

	return 0;
}
