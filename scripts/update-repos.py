#!/usr/bin/python3

import gzip
import os

wwwroot = "/home/packages/www/repos.springrts.com"
prefix = "http://repos.springrts.com/"
repos = {
	"main": "http://packages.springrts.com"
}

for repo in os.listdir(wwwroot):
	absname = os.path.join(wwwroot, repo)
	if not os.path.isdir(absname):
		continue
	assert(repo not in repos)
	repos[repo] = prefix + repo


with gzip.open(os.path.join(wwwroot, "repos.gz"), "wb") as f:
	for reponame, prefix in sorted(repos.items()):
		line = "%s,%s,,\n" %(reponame, prefix)
		f.write(line.encode())

