#
# Regular cron jobs for the example7 package
#
0 4	* * *	root	[ -x /usr/bin/example7_maintenance ] && /usr/bin/example7_maintenance
