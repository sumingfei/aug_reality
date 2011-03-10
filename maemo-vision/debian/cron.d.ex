#
# Regular cron jobs for the maemo-vision package
#
0 4	* * *	root	[ -x /usr/bin/maemo-vision_maintenance ] && /usr/bin/maemo-vision_maintenance
