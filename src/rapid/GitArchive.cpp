/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "GitArchive.h"
#include "Logger.h"
#include "rapid/Hex.h"
#include "rapid/PoolArchive.h"
#include "rapid/PoolFile.h"
#include "rapid/ScopeGuard.h"
#include "rapid/String.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <sys/types.h>
#include <sys/stat.h>

namespace Rapid {

void checkRet(int Error, char const * Message, std::string const & Extra)
{
	git_error const * LogToError;
	char const * LogToErrorMessage = "";
	char const * LogToErrorSpacer = "";

	if (!Error) return;

	if ((LogToError = giterr_last()) != nullptr && LogToError->message != nullptr)
	{
		LogToErrorMessage = LogToError->message;
		LogToErrorSpacer = " - ";
	}

	std::string Concat;
	if (!Extra.empty())
	{
		Concat = concat(Message, " '", Extra, "' [", std::to_string(Error), "]", LogToErrorSpacer, LogToErrorMessage);
	}
	else
	{
		Concat = concat(Message, " [", std::to_string(Error), ']', LogToErrorSpacer, LogToErrorMessage);
	}
	throw std::runtime_error{Concat};
}

CommitInfoT extractVersion(std::string const & Log, std::string const & RevisionString)
{
	std::string const StableString{"STABLE"};
	std::string const VersionString{"VERSION{"};

	if (
		Log.size() >= StableString.size() &&
		std::equal(StableString.begin(), StableString.end(), Log.begin()))
	{
		return {"stable", concat("stable-", RevisionString), true};
	}
	else if (
		Log.size() >= VersionString.size() &&
		std::equal(VersionString.begin(), VersionString.end(), Log.begin()))
	{
		auto First = Log.begin() + VersionString.size();
		auto Last = Log.end();
		auto EndPos = std::find(First, Last, '}');
		if (EndPos != Last) return {"stable", {First, EndPos}, true};
	}

	return {"test", concat("test-", RevisionString), false};
}

void convertTreeishToTree(git_tree** Tree, git_repository* Repo, std::string const & Treeish)
{
	git_object * Object;
	checkRet(git_revparse_single(&Object, Repo, Treeish.c_str()), "git_revparse_single", Treeish);
	auto && ObjectGuard = makeScopeGuard([&] { git_object_free(Object); });
	checkRet(git_object_peel(reinterpret_cast<git_object * *>(Tree), Object, GIT_OBJ_TREE), "git_object_peel");
}

std::string concatPrefix(const std::string& prefix, const std::string& path)
{
	if (prefix.empty())
		return path;

	return concat(prefix, "/", path);
}

typedef std::unordered_map<std::string, std::pair<std::string, std::string>> SubmoduleHashes;
struct SubmoduleContext {
	StoreT& Store;
	PoolArchiveT& Archive;
	const std::string& ModRoot;
	const std::string& pathPrefix;
	SubmoduleHashes submoduleHashes;
};

void processDiff(git_diff *Diff, StoreT& Store, PoolArchiveT& Archive, git_repository* Repo, SubmoduleHashes& submoduleHashes, const std::string pathPrefix)
{
	auto && DiffGuard = makeScopeGuard([&] { git_diff_free(Diff); });

	// Helper function used for adds/modifications
	auto add = [&](git_diff_delta const * Delta, const std::string& fullPath)
	{
		PoolFileT File{Store};
		git_blob * Blob;

		checkRet(git_blob_lookup(&Blob, Repo, &Delta->new_file.id), "git_blob_lookup");
		auto && BlobGuard = makeScopeGuard([&] { git_blob_free(Blob); });
		auto Size = git_blob_rawsize(Blob);
		auto Pointer = static_cast<char const *>(git_blob_rawcontent(Blob));
		File.write(Pointer, Size);
		auto FileEntry = File.close();
		Archive.add(fullPath, FileEntry);
	};

	// Update the archive with this diff
	for (std::size_t I = 0, E = git_diff_num_deltas(Diff); I != E; ++I)
	{
		auto Delta = git_diff_get_delta(Diff, I);
		const std::string& fullPath = concatPrefix(pathPrefix, Delta->new_file.path);

		switch (Delta->status) {
			case GIT_DELTA_ADDED: {
				switch(Delta->new_file.mode) {
					case GIT_FILEMODE_BLOB_EXECUTABLE:
					case GIT_FILEMODE_BLOB: {
						std::cout << "A\t" << fullPath << "\n";
						add(Delta, fullPath);
					} break;
					case GIT_FILEMODE_COMMIT: {
						char buffer[128];
						git_oid_tostr(buffer, 128, &Delta->new_file.id);
						submoduleHashes[std::string(Delta->new_file.path)] = {"" , buffer};
						std::cout << "A\t" << fullPath << " " << buffer << "\n";
					} break;
					default:
						LOG_ERROR("GIT_DELTA_ADDED: Unsupported mode for %s: %d", fullPath.c_str(), Delta->new_file.mode);
						throw std::runtime_error{"Unsupported mode"};

				}
			} break;

			case GIT_DELTA_MODIFIED:
			{
				switch(Delta->new_file.mode) {
					case GIT_FILEMODE_BLOB_EXECUTABLE:
					case GIT_FILEMODE_BLOB: {
						std::cout << "M\t" << fullPath << "\n";
						add(Delta, fullPath);
					} break;
					case GIT_FILEMODE_COMMIT: {
						char buffer1[128];
						char buffer2[128];
						git_oid_tostr(buffer1, 128, &Delta->old_file.id);
						git_oid_tostr(buffer2, 128, &Delta->new_file.id);
						submoduleHashes[std::string(Delta->new_file.path)] = {buffer1 , buffer2};
						std::cout << "M\t" << fullPath << " " << buffer1 << " => " << buffer2 << "\n";
					} break;
					default:
						LOG_ERROR("GIT_DELTA_MODIFIED: Unsupported mode for %s: %d", fullPath.c_str(), Delta->new_file.mode);
						throw std::runtime_error{"Unsupported mode"};

				}
			} break;

			case GIT_DELTA_DELETED:
			{
				switch(Delta->old_file.mode) {
					case GIT_FILEMODE_BLOB_EXECUTABLE:
					case GIT_FILEMODE_BLOB: {
						std::cout << "D\t" << fullPath << "\n";
						Archive.remove(Delta->new_file.path);
					} break;
					case GIT_FILEMODE_COMMIT: {
						std::cout << "D\t" << fullPath << " (submodule)\n";
						Archive.removePrefix(fullPath);
					} break;
					default:
						LOG_ERROR("GIT_DELTA_DELETED: Unsupported mode for %s: %d", fullPath.c_str(), Delta->new_file.mode);
						throw std::runtime_error{"Unsupported mode"};

				}
			} break;

			default: throw std::runtime_error{"Unsupported delta"};
		}
	}
}

void processRepo(StoreT& Store, PoolArchiveT& Archive, git_repository* Repo, const std::string& ModRoot, const std::string& oldHash, const std::string& newHash, const std::string& pathPrefix)
{
	// Diff against the last processed commit tree, or the empty tree if there is none
	git_diff * Diff;
	{
		git_diff_options Options;
		git_diff_options_init(&Options, GIT_DIFF_OPTIONS_VERSION);
		git_tree * SourceTree = nullptr;

		git_tree * DestTree;
		const std::string DestTreeish = concat(newHash, ':', ModRoot);
		convertTreeishToTree(&DestTree, Repo, DestTreeish.c_str());
		auto && DestGuard = makeScopeGuard([&] { git_tree_free(DestTree); });

		auto && SourceGuard = makeScopeGuard([&] { git_tree_free(SourceTree); });

		if (!oldHash.empty()) {
			std::string SourceTreeish = concat(oldHash, ':', ModRoot);;
			convertTreeishToTree(&SourceTree, Repo, SourceTreeish.c_str());
		}
		checkRet(git_diff_tree_to_tree(&Diff, Repo, SourceTree, DestTree, &Options), "git_diff_tree_to_tree");
	}

	SubmoduleContext submoduleContext{Store, Archive, ModRoot, pathPrefix, SubmoduleHashes()};

	processDiff(Diff, Store, Archive, Repo, submoduleContext.submoduleHashes, pathPrefix);

	auto submodule_cb = [](git_submodule *sm, const char *name, void *payload)
	{
		const auto sc = reinterpret_cast<SubmoduleContext*>(payload);
		if (sc->submoduleHashes.find(name) == sc->submoduleHashes.end())
			return 0;

		std::cout << "Entering submodule:\t" << name << "\n";
		git_repository * SubmoduleRepo;
		git_submodule_open(&SubmoduleRepo, sm);
		const auto& hashes = sc->submoduleHashes[std::string(name)];
		std::cout << hashes.first << " " << hashes.second << "\n";

		processRepo(sc->Store, sc->Archive, SubmoduleRepo, sc->ModRoot, hashes.first, hashes.second, concatPrefix(sc->pathPrefix, name));

		return 0;
	};
	git_submodule_foreach(Repo, submodule_cb, &submoduleContext);
}

}

