#!/usr/bin/python3

import glob, os, time


# delete all builds/*.gz files older than 14 days expect the last three

rootdir = "/home/packages/www/repos.springrts.com"

def sizeof_fmt(num, suffix='B'):
	for unit in ['','Ki','Mi','Gi','Ti','Pi','Ei','Zi']:
		if abs(num) < 1024.0:
			return "%3.1f%s%s" % (num, unit, suffix)
		num /= 1024.0
	return "%.1f%s%s" % (num, 'Yi', suffix)

def CleanupBuilds(now, path):
	files = glob.glob(path + "/*.sdz")
	deleted = 0
	count = 0

	files.sort(key=lambda x: os.path.getmtime(x))
	if len(files) < 3:
		#print("To few files to cleanup for %s" %(path))
		return 0, 0
	#print(path)
	for i in range(0, len(files) - 3):
		filename = files[i]
		if os.path.getmtime(filename) > now - 14 * 86400:
			#print(i, filename, "skipped")
			continue
		print(i, filename, "deleting")
		deleted += os.path.getsize(filename)
		count += 1
		os.unlink(filename)

	#for i in range(len(files) - 3, len(files)):
	#	print(i, files[i], "skipped by count")
	return deleted, count


now = time.time()
paths = glob.glob(rootdir + "/*/builds")

deleted = 0
count = 0
for path in paths:
	tdeleted, tcount = CleanupBuilds(now, path)
	count += tcount
	deleted += tdeleted

if deleted > 0:
	print("Deleted", count, sizeof_fmt(deleted))

