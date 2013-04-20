
#include <git2.h>
#include <stdio.h>
#include <string.h>

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


static void check_error(int error_code, const char *action)
{
	if (!error_code)
		return;

	const git_error *error = giterr_last();
	fprintf(stderr, "Error %d %s: %s\n", -error_code, action,
		(error && error->message) ? error->message : "???");
	exit(1);
}

static int push_commit(git_revwalk *walk, const git_oid *oid, int hide)
{
	if (hide)
		return git_revwalk_hide(walk, oid);
	else
		return git_revwalk_push(walk, oid);
}

static int push_spec(git_repository *repo, git_revwalk *walk, const char *spec, int hide)
{
	int error;
	git_object *obj;

	if ((error = git_revparse_single(&obj, repo, spec)) < 0)
		return error;

	error = push_commit(walk, git_object_id(obj), hide);
	git_object_free(obj);
	return error;
}

static int push_range(git_repository *repo, git_revwalk *walk, const char *range, int hide)
{
	git_revspec revspec;
	int error = 0;

	if ((error = git_revparse(&revspec, repo, range)))
		return error;

	if (revspec.flags & GIT_REVPARSE_MERGE_BASE) {
		/* TODO: support "<commit>...<commit>" */
		return GIT_EINVALIDSPEC;
	}

	if ((error = push_commit(walk, git_object_id(revspec.from), !hide)))
		goto out;

	error = push_commit(walk, git_object_id(revspec.to), hide);

out:
	git_object_free(revspec.from);
	git_object_free(revspec.to);
	return error;
}

/*
static int revwalk_parseopts(git_repository *repo, git_revwalk *walk, int nopts, char **opts)
{
  int hide, i, error;
  unsigned int sorting = GIT_SORT_NONE;

  hide = 0;
  for (i = 0; i < nopts; i++) {
    if (!strcmp(opts[i], "--topo-order")) {
      sorting = GIT_SORT_TOPOLOGICAL | (sorting & GIT_SORT_REVERSE);
      git_revwalk_sorting(walk, sorting);
    } else if (!strcmp(opts[i], "--date-order")) {
      sorting = GIT_SORT_TIME | (sorting & GIT_SORT_REVERSE);
      git_revwalk_sorting(walk, sorting);
    } else if (!strcmp(opts[i], "--reverse")) {
      sorting = (sorting & ~GIT_SORT_REVERSE)
          | ((sorting & GIT_SORT_REVERSE) ? 0 : GIT_SORT_REVERSE);
      git_revwalk_sorting(walk, sorting);
    } else if (!strcmp(opts[i], "--not")) {
      hide = !hide;
    } else if (opts[i][0] == '^') {
      if ((error = push_spec(repo, walk, opts[i] + 1, !hide)))
        return error;
    } else if (strstr(opts[i], "..")) {
      if ((error = push_range(repo, walk, opts[i], hide)))
        return error;
    } else {
      if ((error = push_spec(repo, walk, opts[i], hide)))
        return error;
    }
  }

  return 0;
}
*/

git_repository *openrepo(const char* path)
{
	int error;
	git_revwalk *walk;
	git_oid oid;
	char buf[41];
	git_repository *repo;

	error = git_repository_open_ext(&repo, path, 0, NULL);
	check_error(error, "opening repository");

	error = git_revwalk_new(&walk, repo);
	check_error(error, "allocating revwalk");
//	error = revwalk_parseopts(repo, walk, argc-1, argv+1);
//	check_error(error, "parsing options");
	return repo;

}

/*
	params
	./build-ca "file://$REPO" trunk/mods trunk/mods/zk/modinfo.lua /home/packages/packages $REVISION zk
*/

/*TODO:
	1. parse params
	2. get revision number (=commit hash?!) of packages folder for this repo
	3. get revision numbers of git repo
	4. foreach commit (diff of revisions in repo / revisions in packages)
		get modinfo.lua
		replace version
		get all files for commit hash
		write all files to packages excluding modinfo
		write modified modinfo to packages
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

int main (int argc, char** argv)
{
	char *dir = ".";
	if (argc > 1)
		dir = argv[1];
	if (!dir || argc > 2) {
		fprintf(stderr, "usage: showindex [<repo-dir>]\n");
		return 1;
	}
	git_repository* repo;
	repo = openrepo(dir);
	listfiles(repo);
	return 0;
}

