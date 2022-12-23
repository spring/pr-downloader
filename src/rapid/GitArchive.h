/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */
#pragma once

#include <git2.h>
#include <string>
#include <algorithm>

#include "rapid/Store.h"
#include "rapid/PoolArchive.h"

namespace Rapid {

//using namespace Rapid;

struct CommitInfoT
{
	std::string Branch;
	std::string Version;
	bool MakeZip;
};

void checkRet(int Error, char const * Message, std::string const & Extra = "");
CommitInfoT extractVersion(std::string const & Log, std::string const & RevisionString);
void convertTreeishToTree(git_tree** Tree, git_repository* Repo, std::string const & Treeish);
void processRepo(StoreT& Store, PoolArchiveT& Archive, git_repository* Repo, const std::string& ModRoot, const std::string& oldHash, const std::string& newHash, const std::string& pathPrefix);

// Replace all instances of $VERSION in a string by calling a functor with substrings
template<typename FunctorT>
void replaceVersion(char const * Buffer, std::size_t BufferSize, std::string const & Replacement, FunctorT Functor)
{
	auto Pos = Buffer;
	auto Last = Buffer + BufferSize;
	std::string VersionString{"$VERSION"};

	while (true)
	{
		auto NewPos = std::search(Pos, Last, VersionString.begin(), VersionString.end());
		if (NewPos == Last) break;
		auto Size = NewPos - Pos;
		if (Size == 0) continue;
		Functor(Pos, Size);
		Functor(Replacement.data(), Replacement.size());
		Pos = NewPos + VersionString.size();
	}

	auto Size = Last - Pos;
	if (Size == 0) return;
	Functor(Pos, Size);
}

} // namespace

