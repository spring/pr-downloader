#!/bin/bash

ionice -p $$ -c 3
renice 20 -p $$ >/dev/null

MAXLOAD=3
LOAD=$(uptime | egrep -o -e "load average: [0-9]*"|cut -b 15-)
if [ $LOAD -ge $MAXLOAD ]; then
	echo current load $LOAD is higher than maxload $MAXLOAD, aborting sync
	exit
fi


# checks all 
MODROOT=/
MODINFO=modinfo.lua
PACKAGES=/home/packages/www/repos.springrts.com

REPOS=$(find /home/packages/git -maxdepth 1 -mindepth 1 -type d)


for REPO in $REPOS; do
	cd $REPO
	if git fetch --prune &>/dev/null; then
		LOCAL=$(git rev-parse HEAD)
		REMOTE=$(git rev-parse @{u})
		if [ "$LOCAL" != "$REMOTE" ]; then
			TAG=$(basename $REPO)
			(
			echo Stated: $(date)
			echo Updating $REPO from $(git config --get remote.origin.url)
			git pull
			git checkout master
			git reset --hard origin/master
			git submodule sync --recursive
			git submodule update --recursive --remote --init
			~/bin/BuildGit "$REPO" "$MODROOT" "$MODINFO" "$PACKAGES/$TAG" "$REMOTE" "$TAG"
			echo Finished: $(date)
			) &> $PACKAGES/$TAG/log.txt
			git log -1 --pretty=format:"%an commited %h: %s" | ~/bin/loggit.py "$TAG"
		fi
	fi
done

