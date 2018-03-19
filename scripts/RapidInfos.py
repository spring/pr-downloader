#!/usr/bin/env python

#prints all stable / test tag in a rapid dir

import gzip,sys,os

# returns all stable tags
def getTags(filename):
	tags = {}
	lines = gzip.open(filename).readlines()
	for line in lines:
		line = line.strip("\r\n")
		items = line.split(",")
		if len(items) < 1:
			continue
		tag = items[0]
		if len(tag.split(':')) == 3:
			continue
		tags[items[0]] = items[3]
	return tags

def getfiles(dirname):
	tags = {}
	for f in os.listdir(dirname):
		if f != "versions.gz":
			continue
		filename = os.path.join(dirname, f)
		if os.path.isfile(filename):
			tags.update(getTags(filename))
	return tags

rootdir = sys.argv[1]
tags = {}
for d in os.listdir(rootdir):
	repodir = os.path.join(rootdir, d)
	if os.path.isdir(repodir):
		tags.update(getfiles(repodir))

print("<table>")
print("<tr><td>Tag</td><td>Name</td></tr>")
for tag in sorted(tags.items()):
	print("<tr><td>%s</td><td>%s</td></tr>" %(tag[0], tag[1]))
print("</table>")

