/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Logger.h"
#include "rapid/GitArchive.h"
#include "rapid/Hex.h"
#include "rapid/Last.h"
#include "rapid/LastGit.h"
#include "rapid/LastGit.h"
#include "rapid/PoolArchive.h"
#include "rapid/PoolFile.h"
#include "rapid/ScopeGuard.h"
#include "rapid/Store.h"
#include "rapid/String.h"
#include "rapid/Versions.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <git2.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace {

using namespace Rapid;

void buildGit(
	std::string const & GitPath,
	std::string const & ModRoot,
	std::string const & Modinfo,
	std::string const & StorePath,
	std::string const & GitHash,
	std::string const & Prefix)
{
	// Initialize libgit2
	checkRet(git_libgit2_init(), "git_libgit2_init()");
	auto && ThreadsGuard = makeScopeGuard([&] { git_libgit2_shutdown(); });

	// Load the git repo
	git_repository * Repo;
	checkRet(git_repository_open_ext(&Repo, GitPath.c_str(), 0, nullptr), "git_repository_open_ext");
	auto && RepoGuard = makeScopeGuard([&] { git_repository_free(Repo); });

	// Lookup destination oid
	git_oid DestOid;
	checkRet(git_oid_fromstr(&DestOid, GitHash.c_str()), "git_oid_fromstr");

	// Make a short hash
	std::array<char, 7> ShortHash;
	std::copy(GitHash.data(), GitHash.data() + 7, ShortHash.data());

	// Find the commit count
	git_revwalk * Walker;
	checkRet(git_revwalk_new(&Walker, Repo), "git_revwalk_new");
	auto && WalkerGuard = makeScopeGuard([&] { git_revwalk_free(Walker); });
	checkRet(git_revwalk_push(Walker, &DestOid), "git_revwalk_push");

	std::size_t CommitCount = 0;
	while (true)
	{
		git_oid WalkerOid;
		int Ret = git_revwalk_next(&WalkerOid, Walker);
		if (Ret == GIT_ITEROVER) break;
		checkRet(Ret, "git_revwalk_next");
		++CommitCount;
	}

	// Extract the commit type from the commit message
	git_commit * Commit;
	checkRet(git_commit_lookup(&Commit, Repo, &DestOid), "git_commit_lookup");
	auto && CommitGuard = makeScopeGuard([&] { git_commit_free(Commit); });
	//std::size_t AncestorCount = git_commit_parentcount(Commit);
	std::string TestVersion = concat(std::to_string(CommitCount), '-', ShortHash);
	auto CommitInfo = extractVersion(git_commit_message_raw(Commit), TestVersion);

	// Initialize the store
	StoreT Store{StorePath};
	Store.init();

	// Load the destination commit tree
	git_tree * DestTree;
	std::string const DestTreeish = concat(GitHash, ':', ModRoot);
	convertTreeishToTree(&DestTree, Repo, DestTreeish.c_str());
	auto && DestGuard = makeScopeGuard([&] { git_tree_free(DestTree); });
	// Prepare to perform diff
	PoolArchiveT Archive{Store};

	auto Option = LastGitT::load(Store, Prefix);
	std::string oldHash;

	if (!Option) {
			std::cout << "Unable to perform incremental an update\n";
	} else {
		auto & Last = *Option;
		Archive.load(Last.Digest);
		oldHash.resize(40);
		Hex::encode(&oldHash[0], Last.Hex.data(), 20);

		std::cout <<
			"Performing incremental update: " <<
			oldHash <<
			"..." <<
			GitHash <<
			"\n";
	}
	processRepo(Store, Archive, Repo, ModRoot, oldHash, GitHash, "");


	// Update modinfo.lua with $VERSION replacement
	git_tree_entry * TreeEntry;
	checkRet(git_tree_entry_bypath(&TreeEntry, DestTree, Modinfo.c_str()), "git_tree_entry_bypath");
	git_blob * Blob;
	checkRet(git_blob_lookup(&Blob, Repo, git_tree_entry_id(TreeEntry)), "git_blob_lookup");
	auto && BlobGuard = makeScopeGuard([&] { git_blob_free(Blob); });
	auto Size = git_blob_rawsize(Blob);
	auto Pointer = static_cast<char const *>(git_blob_rawcontent(Blob));
	PoolFileT File{Store};
	replaceVersion(Pointer, Size, CommitInfo.Version, [&](char const * Data, std::size_t Length)
	{
		File.write(Data, Length);
	});
	auto FileEntry = File.close();
	Archive.add(Modinfo, FileEntry);
	auto ArchiveEntry = Archive.save();

	// Add tags and save versions.gz
	VersionsT Versions{Store};
	Versions.load();
	std::string const Tag = concat(Prefix, ':', CommitInfo.Branch);
	std::string const Tag2 = concat(Prefix, ":git:", GitHash);
	Versions.add(Tag, ArchiveEntry);
	Versions.add(Tag2, ArchiveEntry);
	Versions.save();

	// Save info for next incremental run
	LastGitT Last;
	Hex::decode(GitHash.c_str(), Last.Hex.data(), 20);
	Last.Digest = ArchiveEntry.Digest;
	LastGitT::save(Last, Store, Prefix);

	// Create zip if needed
	if (CommitInfo.MakeZip)
	{
		auto Path = Store.getBuildPath(Prefix, CommitInfo.Version);
		std::cout << "Generating zip file: " << Path << "\n";
		Archive.makeZip(Path);
		// Call upload.py?
	}
}

} // namespace

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
		buildGit(
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
