#!/bin/bash

# disable malloc check in glibc otw. we get corruption blarg on exit() with NDEBUG builds
export MALLOC_CHECK_=0

declare -i TIMEOUT=100
declare -i MEMOUT=70

export GNUTIME="/usr/bin/time --verbose -o" # time command
export RUN="run -s $MEMOUT -t $((TIMEOUT+20)) -k -o"
export TIMELIMIT="timelimit -p -s 1 -t $TIMEOUT -T 20"
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S' # time format
export TESTSPATH='.' # path to lp/br/opt
export DMCSPATH='../../../build/src' # path to dmcsd/dmcsc
export LOGPATH='.' # path to output logfiles

DORUN=no # run with `run'
DOTIMELIMIT=no # run with `timelimit'
VERBOSE=no # output stuff
LOGDAEMONS=no # log daemon output

# test instance
export TOPO=tree
declare -i CTX=200
declare -i SIG=9
declare -i BRS=4
declare -i RLS=4
export INST=b

export MODE=streaming
export PACK=5 # report at most $PACK many equilibria

################### below we don't need to touch anything

# check GNU time
$GNUTIME /dev/null ls > /dev/null 2>&1
if [ $? != 0 ]; then
	echo I need GNU time, please setup GNUTIME properly. Bailing out.
	exit 1
fi

export TESTNAME=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST
declare -i MINPORT=5000

export DMCSD=$DMCSPATH/dmcsd
export DMCSC=$DMCSPATH/dmcsc


STATSLOG=$LOGPATH/$TESTNAME-$MODE-status.log

date > $STATSLOG

declare -i N
for (( N = 1 ; N <= $CTX ; N++ )); do

	INPUT=$TESTSPATH/$TESTNAME
	INPUTN=$TESTSPATH/$TESTNAME-$N

	LOGN=/dev/null
	RUNLOGN=/dev/null
	TIMELOGN=/dev/null
	if [ x$LOGDAEMONS = xyes ] ; then
		LOGN=$LOGPATH/$TESTNAME-$MODE-$N.log
		RUNLOGN=$LOGPATH/$TESTNAME-$MODE-run-$N.log
		TIMELOGN=$LOGPATH/$TESTNAME-$MODE-time-$N.log
	fi

	DMCSDOPTS="--context=$N --port=$((MINPORT+N)) --kb=$INPUTN.lp --br=$INPUTN.br --topology=$INPUT.opt"

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

sleep 40

if [ x$MODE = xstreaming ] ; then
    LOGTIME=$LOGPATH/$TESTNAME-$MODE-$PACK-time.log
    LOGCOUT=$LOGPATH/$TESTNAME-$MODE-$PACK.log
    LOGCERR=$LOGPATH/$TESTNAME-$MODE-$PACK-err.log
    LOGRUN=$LOGPATH/$TESTNAME-$MODE-$PACK-run.log
else
    LOGTIME=$LOGPATH/$TESTNAME-$MODE-time.log
    LOGCOUT=$LOGPATH/$TESTNAME-$MODE.log
    LOGCERR=$LOGPATH/$TESTNAME-$MODE-err.log
    LOGRUN=$LOGPATH/$TESTNAME-$MODE-run.log
fi

DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1)) --system-size=$CTX"
MODEOPTS="--s=0"
if [ x$MODE = xstreaming ] ; then
    MODEOPTS="--s=1 --k=$PACK"
fi

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
	echo PASS: $TESTNAME-$MODE >> $STATSLOG
else
	egrep -H "status" $LOGPATH/$TESTNAME-$MODE-run*.log >> $STATSLOG
	echo FAILED: $TESTNAME-$MODE >> $STATSLOG
fi

echo >> $STATSLOG

date >> $STATSLOG

cat $STATSLOG
