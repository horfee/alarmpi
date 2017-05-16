#!/bin/sh
#
# Start the alarm system....
#
umask 077

$pidfile = 
case "$1" in
  start)
	printf "Starting alarm: "
	/usr/bin/alarmPI
	[ $? = 0 ] && echo "OK" || echo "FAIL"
	;;
  stop)
	printf "Stopping alarm: "
	killall alarmPI
	[ $? = 0 ] && echo "OK" || echo "FAIL"
	;;
  restart|reload)
	"$0" stop
	"$0" start
	;;
  *)
	echo "Usage: $0 {start|stop|restart}"
	exit 1
esac

exit $?
