#!/usr/bin/python3

# updates repos.gz

import gzip
import os

wwwroot = "/home/packages/www/repos.springrts.com"
prefix = "http://repos.springrts.com/"
streamer = "/home/packages/bin/Streamer"
repos = {
	"main": "http://packages.springrts.com"
}

assert(os.path.isfile(streamer))

def SetupStreamer(repodir):
	streamercgi = os.path.join(repodir, "streamer.cgi")
	if os.path.islink(streamercgi):
		if os.readlink(streamercgi) == streamer:
			return
		print("Deleted invalid link %s" %(streamercgi))
		os.unlink(streamercgi)
	print("Created symlink %s -> %s" %(streamercgi, streamer))
	os.symlink(streamer, streamercgi)


def GetWWWRepos(wwwroot):
	for repo in os.listdir(wwwroot):
		absname = os.path.join(wwwroot, repo)
		if not os.path.isdir(absname):
			continue
		assert(repo not in repos)
		repos[repo] = prefix + repo
	return repos

def ParseRepos(reposgz):
	res = {}
	with gzip.open(os.path.join(wwwroot, "repos.gz"), "rb") as f:
		for line in f.readlines():
			name, url, _, _ = line.decode().split(",", 4)
			res[name] = url
	return res

def WriteRepos(reposgz, repos):
	with gzip.open(reposgz, "wb") as f:
		for reponame, prefix in sorted(repos.items()):
			line = "%s,%s,,\n" %(reponame, prefix)
			f.write(line.encode())

def SetupRepos(wwwroot, wwwrepos):
	for reponame in wwwrepos:
		absdir = os.path.join(wwwroot, reponame)
		if os.path.isdir(absdir):
			SetupStreamer(absdir)

def ReposEqual(repoa, repob):
	for a, b in (repoa, repob), (repob, repoa):
		for k, v in a.items():
			if not k in b:
				return False
			if v != b[k]:
				return False
	return True



reposgz = os.path.join(wwwroot, "repos.gz")
repos.update(GetWWWRepos(wwwroot))

SetupRepos(wwwroot, repos)

currentrepos = ParseRepos(reposgz)

if not ReposEqual(repos, currentrepos):
	print("Updating %s" %(reposgz))
	WriteRepos(reposgz, repos)

assert(ReposEqual(ParseRepos(reposgz), repos))




