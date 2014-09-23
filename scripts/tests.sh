#!/bin/sh
export PATH=/bin:/usr/bin:/sbin

if [ "$UID" = "0" ] ; then
    echo '**** WARNING ****    Do not start this script as ROOT !!!'
    echo 'Please, interrupt me!  Otherwise I continue in 30 sec.'
    sleep 30
fi

h=`pwd`
home=$h/home
export TIMEFORMAT='  elapsed time: %R'
export NINIT_HOME=$home

cstart='' ; cend='' ; bad=''
tty -s 2>/dev/null
if [ $? -eq 0 ] ; then
    # 30=black 31=red 32=green 33=yellow 34=blue ... 37=white
    # 40=black 41=red ... backgrownd color
    cstart='[1;47m[1;30m' ; cend='[0;39m'; bad='[1;44m[1;31m'
fi

echo '  starting server' $cstart$h/ninit -H$home$cend
echo '  wait for this test more than 25 seconds'
echo '  no PANIC!  the test terminates in 92 sec always' 
echo 

start_time=`date +%s`

print_env () {
    if [ -f /proc/$ninit_pid/environ ] ; then
	echo
	echo '  '$1 $cstart \
	    `./sleeprun -a4 tr '\000' ' ' < /proc/$ninit_pid/environ` $cend
    fi
}

do_it () {
    X=$1; shift
    echo '  starting '$cstart$@$cend'	#' $X
    ./sleeprun -a20 $@
    ret=$?
    if [ $ret -ne 0 ] ; then
	echo $bad'***** WARNING *****'$cend
	echo 'Exit status of' $cstart$@$cend is $ret
	sleep 3
    fi
    return $ret
}

do_it '(install HOME)' ./install-bin home < misc/HOME || exit 1
test -f home/env/run || gzip -dc home.tar.gz | do_it '(unpack home)' tar -xv

args="./sleeprun -a90 ./env -i NINIT_TIME=empty $h/ninit -H$home"
echo '  starting' $cstart$args$cend
$args &
ninit_pid=$!
echo '  ninit PID='$cstart$ninit_pid$cend

do_it '(waiting the ninit to start)' sleep 1
do_it '(list)'			./nsvc -L
time do_it '(wait 4s env)'	./nsvc -W4 env
do_it '(once)'			./nsvc -o sleep
do_it '(set respawning mode)'	./nsvc -R sleep
do_it '(list)'			./nsvc -L

sleep 1
do_it '(diagnostic)'	./nsvc sleep ; echo
do_it '(list)'		./nsvc -L
do_it '(list pids)'	./nsvc -g sleep default
do_it '(history)'	./nsvc -H

echo ; echo '  reloading ninit'

after=after-reload
do_it ''	ps uwwww -p $ninit_pid ; echo
do_it ''	ln -sf ninit $after

print_env 'old ninit environ:'

current_time=`date +%s`
args="./reload -v -a 20 -eNINIT_TIME=$current_time -e Hello=world"
args="$args -eYou_are=$USER -u $h/$after -H$home"
echo '  starting' $cstart$args$cend
time ./sleeprun -a30 $args

print_env 'new ninit environ:'

do_it '(sleep 4+3 sec;  stopping do_update mode)' sleep 4
do_it ''	ps uwwww -p $ninit_pid ; echo
do_it ''	sleep 3

echo
time do_it 'sync mode (max 3 sec)'	./nsvc -o S
do_it '(list)'				./nsvc -L
all=`./nsvc -L`
do_it '(remove cron flags)'		./nsvc -K0 $all
do_it '(down all services)'		./nsvc -d $all

time do_it '(wait 1s all to finish)'	./nsvc -W1 $all
do_it '(list)'		./nsvc -L
do_it '(memory usage)'	./nsvc -D
do_it '(depends)'	./nsvc -D default

[ -f /proc/$ninit_pid/statm ] && \
    do_it '(ninit daemon)' cat /proc/$ninit_pid/statm
do_it '(kill ninit daemon)'	kill $ninit_pid
wait

end_time=`date +%s`
./sleeprun -a4 rm -f $after

n=`expr $end_time - $start_time` ; echo
echo '  test continues:' $cstart$n sec$cend

echo
echo $cstart'    Creating services in ./etc/ninit - Demo    '$cend
do_it '(clean old ./etc/)' rm -r -f ./etc
do_it '(install ./etc/ninit/)' ./install-bin . < misc/ETC
do_it '(converter)' ./inittab /etc/inittab etc/ninit services.sh > /dev/null
do_it '(making services - demo)' ./services.sh
# do_it '' ls -F ./etc/ninit

echo
echo Look now at $cstart./services.sh$cend and $cstart./etc/ninit/$cend.
echo
