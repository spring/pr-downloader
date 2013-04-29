
#include <git2.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

bool listfiles(git_repository *repo)
{
	git_index *index;
	unsigned int i, ecount;
	char out[41];
	out[40] = '\0';



	if (git_repository_index(&index, repo) < 0) {
		fprintf(stderr, "could not open repository index\n");
		return false;
	}

	git_index_read(index);

	ecount = git_index_entrycount(index);
	if (!ecount)
		printf("Empty index\n");

	for (i = 0; i < ecount; ++i) {
		const git_index_entry *e = git_index_get_byindex(index, i);

		git_oid_fmt(out, &e->oid);

		printf("File Path: %s\n", e->path);
		printf("    Stage: %d\n", git_index_entry_stage(e));
		printf(" Blob SHA: %s\n", out);
		printf("File Mode: %07o\n", e->mode);
		printf("File Size: %d bytes\n", (int)e->file_size);
		printf("Dev/Inode: %d/%d\n", (int)e->dev, (int)e->ino);
		printf("  UID/GID: %d/%d\n", (int)e->uid, (int)e->gid);
		printf("    ctime: %d\n", (int)e->ctime.seconds);
		printf("    mtime: %d\n", (int)e->mtime.seconds);
		printf("\n");
	}

	git_index_free(index);
	git_repository_free(repo);
}


void check_error(int error_code, const char *action)
{
	if (!error_code)
		return;

	const git_error *error = giterr_last();
	fprintf(stderr, "Error %d %s: %s\n", -error_code, action,
		(error && error->message) ? error->message : "???");
	exit(1);
}


int push_spec(git_repository *repo, git_revwalk *walk, const char *spec)
{
	int error;
	git_object *obj;

	if ((error = git_revparse_single(&obj, repo, spec)) < 0)
		return error;

	error = git_revwalk_push(walk, git_object_id(obj));
	git_object_free(obj);
	return error;
}

int push_range(git_repository *repo, git_revwalk *walk, const char *range)
{
	git_revspec revspec;
	int error = 0;

	if ((error = git_revparse(&revspec, repo, range)))
		return error;

	if (revspec.flags & GIT_REVPARSE_MERGE_BASE) {
		/* TODO: support "<commit>...<commit>" */
		return GIT_EINVALIDSPEC;
	}

	error = git_revwalk_push(walk, git_object_id(revspec.from));

	if (!error)
		error = git_revwalk_push(walk, git_object_id(revspec.to));

	git_object_free(revspec.from);
	git_object_free(revspec.to);
	return error;
}


void listrevisions(git_repository *repo, std::vector<git_oid>& oids)
{
	int error;
	const unsigned int sorting = GIT_SORT_TOPOLOGICAL|GIT_SORT_REVERSE;
	git_revwalk *walk;

	error = git_revwalk_new(&walk, repo);
	check_error(error, "allocating revwalk");

	git_revwalk_sorting(walk, sorting);
	push_spec(repo, walk, "HEAD");
	git_oid oid;
	while (!git_revwalk_next(&oid, walk)) {
		oids.push_back(oid);
	}
}


git_repository *openrepo(const std::string& path)
{
	int error;
	git_oid oid;
	char buf[41];
	git_repository *repo;

	error = git_repository_open_ext(&repo, path.c_str(), 0, NULL);
	check_error(error, "opening repository");
	return repo;

}

/*TODO:
	4. foreach commit (diff of revisions in repo / revisions in packages)
		get modinfo.lua
		replace version
		get all files for commit hash
		write all files to packages excluding modinfo
		write modified modinfo to packages
		calc md5 hash of package
		write sdp
		read versions.gz & add created sdp
		write new last version written

	@param repo	path to git repository
	@param logpath	path inside the git repository for log
	@param modinfo	path inside the git repo to modinfo.lua / root path of the .sdd
	@param packages path to the packages folder
	@param revision revision to create sdp
	@param tag      tag name of the game
*/
std::string getCurrentPackageRevision(const std::string& packagespath, const std::string& tag)
{
	printf("Using %s, tag %s\n", packagespath.c_str(), tag.c_str());
	const std::string lastpath = packagespath +"/" + tag;
	FILE* f=fopen(lastpath.c_str(), "rb");
	if (f==NULL)
		return "";
	char buf[1024];
	int count = fread(buf,sizeof(buf), 1, f);
	fclose(f);
	return std::string(buf, count);
}

void printoid(git_oid* oid)
{
	char buf[41];
	buf[40]=0;
	git_oid_fmt(buf, oid);
	printf("oid: %s\n", buf);
}

int main (int argc, char** argv)
{
	if (argc != 7) {
		printf("Usage: %s /path/to/git/repo repo/log/path repo/path/modinfo.lua /path/to/packages <revision> <tag>\n", argv[0]);
		return 1;
	}
	const std::string repopath = argv[1];
	const std::string logpath = argv[2];
	const std::string modinfopath = argv[3];
	const std::string packagespath = argv[4];
	const std::string revision = argv[5];
	const std::string tag = argv[6];

	getCurrentPackageRevision(packagespath, tag);

	git_repository* repo = openrepo(repopath);

	std::vector<git_oid> oids;
	listrevisions(repo, oids);
	if (oids.empty()) {
		printf("no revisions found!\n");
		return 1;
	}
	printoid(&oids[0]);

	int error;

	for (int i=0; i<oids.size(); i++) {
		printf("revision %d: ", i+1);
		printoid(&oids[i]);

		git_commit* lastcommit;
		error = git_commit_lookup(&lastcommit, repo, &oids[i]);
		check_error(error, "git_commit_lookup");
		const char *author =  git_commit_message(lastcommit);
		printf("%s\n", author);
		git_commit_free(lastcommit);
	}


	//listfiles(repo);
	return 0;
}

