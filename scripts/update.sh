#!/bin/sh
# $1 = $(DESTDIR)

D=$1
[ "$D" = "/" ] && D=

echo
echo I will try to overwrite $D/sbin/ninit
echo If PID 1 is $D/sbin/ninit I will get probably an error

[ -w $D/sbin/ninit ] || chmod 755 $D/sbin/ninit 2>/dev/null
./install-bin $D/sbin 'c:::755:/:ninit::' && exit 0

echo I got an error.  I suppose PID 1 is $D/sbin/ninit
echo
echo I will try to replace $D/sbin/ninit with `pwd`/ninit
echo Please wait at least 30 seconds

./sleeprun -a10  $D/sbin/ninit-reload -d > ninit.data && \
  ./sleeprun -a5 $D/sbin/ninit-reload -f ninit.data -u `pwd`/ninit && \
  ./install-bin  $D/sbin 'c:::755:/:ninit::' && \
  ./sleeprun -a8 $D/sbin/ninit-reload -u $D/sbin/ninit && \
  exit 0

echo 'All fails!  I will try simply ./install-bin'

./install-bin $D/sbin 'x:::755:/:ninit::'
