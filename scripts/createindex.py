import os
import json
from lupa import LuaRuntime
from configparser import ConfigParser

def get_remote(repodir):
	configfile = os.path.join(repodir, ".git/config")
	c = ConfigParser()
	c.read(configfile)
	return c.get('remote "origin"', "url")

def get_name(repodir):
	modinfo = os.path.join(repodir, "modinfo.lua")
	lua = LuaRuntime()
	with open(modinfo, "r") as f:
		content = f.read()
		l = lua.execute(content)
		return l["name"]

repolist = {}

for d in os.listdir("/home/packages/git"):
	if not os.path.isdir(d):
		continue
	abspath = os.path.abspath(d)
	reponame = os.path.basename(abspath)
	url = get_remote(abspath)
	name = get_name(abspath)
	assert not reponame in repolist 
	repolist[reponame] = {
		"names": [name],
		"url": url,
		"stablebranch": "master",
		"testbranches": ["master"],
	}


repos = {
	"repos": repolist
}

with open("/home/packages/www/repos.springrts.com/repos.json", "w") as f:
	f.write(json.dumps(repos, indent='\t', sort_keys=True))
