#!/bin/bash

# disable malloc check in glibc otw. we get corruption blarg on exit() with NDEBUG builds
export MALLOC_CHECK_=0

declare -i TIMEOUT=100
declare -i MEMOUT=1000

export GNUTIME="/usr/bin/time --verbose -o" # time command
export RUN="run -s $MEMOUT -t $((TIMEOUT+20)) -k -o"
export TIMELIMIT="timelimit -p -s 1 -t $TIMEOUT -T 20"
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S' # time format
export TESTSPATH='.' # path to lp/br/opt
export DMCSPATH='../build/src' # path to dmcsd/dmcsc
export LOGPATH='.' # path to output logfiles
export RESULTPATH='./result'

DORUN=yes # run with `run'
DOTIMELIMIT=yes # run with `timelimit'
VERBOSE=no # output stuff
LOGDAEMONS=yes # log daemon output

# test instance
export TOPO=random
declare -i POOL=100 # pool of contexts
declare -i RES=3 # max no. of answers
declare -i BND=8 # max binding for rules
declare -i HEU=4 # max heuristics
export INST=a

export MODE=dyn
export PACK=0 # report at most $PACK many equilibria

################### below we don't need to touch anything

# check GNU time
$GNUTIME /dev/null ls > /dev/null 2>&1
if [ $? != 0 ]; then
    echo I need GNU time, please setup GNUTIME properly. Bailing out.
    exit 1
fi

TESTNAME="$TOPO-$POOL-$INST"
declare -i MINPORT=5000

export DMCSD=$DMCSPATH/dmcsd
export DMCSC=$DMCSPATH/dmcsc

STATSLOG=$LOGPATH/$TESTNAME-$MODE-status.log

date > $STATSLOG

declare -i H
declare -i N

for (( H = 0 ; H <= $HEU ; H++ )); do

    for (( N = 1 ; N <= $POOL ; N++ )); do

	INPUT=$TESTSPATH/$TESTNAME
	INPUTN=$TESTSPATH/$TESTNAME-$N

	LOGN=/dev/null
	RUNLOGN=/dev/null
	TIMELOGN=/dev/null
	if [ x$LOGDAEMONS = xyes ] ; then
	    LOGN=$LOGPATH/$TESTNAME-$MODE$H-$N.log
	    RUNLOGN=$LOGPATH/$TESTNAME-$MODE$H-run-$N.log
	    TIMELOGN=$LOGPATH/$TESTNAME-$MODE$H-time-$N.log
	fi
	
	DMCSDOPTS="--context=$N --port=$((MINPORT+N)) --kb=$INPUTN.lp --br=$INPUTN.br --mm=$INPUT.inp --n=$RES --b=$BND --h=$H --dynamic=1"
	
	DMCSDRUN="$DMCSD $DMCSDOPTS"
	
	if [ x$DORUN = xyes ] ; then
	    DMCSDRUN="$RUN $RUNLOGN $DMCSDRUN"
	fi
	
	if [ x$DOTIMELIMIT = xyes ] ; then
	    DMCSDRUN="$TIMELIMIT $DMCSDRUN"
	fi
	
	if [ x$VERBOSE = xyes ] ; then
	    set -x
	fi
	
	$GNUTIME $TIMELOGN $DMCSDRUN > $LOGN 2>&1 &
	
	set +x
	
	disown $! # don't record jobid, so we don't report kill messages

    done

    sleep 3

    LOGTIME=$LOGPATH/$TESTNAME-$MODE$H-time.log
    LOGCOUT=$LOGPATH/$TESTNAME-$MODE$H.log
    LOGCERR=$LOGPATH/$TESTNAME-$MODE$H-err.log
    LOGRUN=$LOGPATH/$TESTNAME-$MODE$H-run.log
    
    DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1))"
    MODEOPTS="--dynamic=1"
    #if [ x$MODE = xstreaming ] ; then
    #MODEOPTS="--s=1 --k=$PACK"
    #fi
    
    DMCSCRUN="$DMCSC $DMCSCOPTS $MODEOPTS"
    
    if [ x$DORUN = xyes ] ; then
	DMCSCRUN="$RUN $LOGRUN $DMCSCRUN"
    fi
    
    if [ x$DOTIMELIMIT = xyes ] ; then
	DMCSCRUN="$TIMELIMIT $DMCSCRUN"
    fi
    
    if [ x$VERBOSE = xyes ] ; then
	set -x
    fi

    $GNUTIME $LOGTIME $DMCSCRUN > $LOGCOUT 2> $LOGCERR

    DMCSCRET=$?

    set +x

    # just to be safe
    killall -1 dmcsd # 2>/dev/null
    killall -9 dmcsd 2>/dev/null
    
    echo >> $STATSLOG

    if [ $DMCSCRET = 0 ]; then
	echo PASS: $TESTNAME-$MODE$H >> $STATSLOG
    else
	egrep -H "status" $LOGPATH/$TESTNAME-$MODE$H-run*.log >> $STATSLOG
	echo FAILED: $TESTNAME-$MODE$H >> $STATSLOG
    fi

    echo >> $STATSLOG

    date >> $STATSLOG

done

cat $STATSLOG
