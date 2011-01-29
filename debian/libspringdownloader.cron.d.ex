#
# Regular cron jobs for the libspringdownloader package
#
0 4	* * *	root	[ -x /usr/bin/libspringdownloader_maintenance ] && /usr/bin/libspringdownloader_maintenance
