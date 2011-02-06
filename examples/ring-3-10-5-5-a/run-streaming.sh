#!/bin/bash

# disable malloc check in glibc otw. we get corruption blarg on exit() with NDEBUG builds
export MALLOC_CHECK_=0

export GNUTIME='/usr/bin/time --portability -o' # time command
export RUN='run -s 1000 -t 180 -k -o'
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S' # time format
export TESTSPATH='.' # path to lp/br/opt
export DMCSPATH='../../build/src' # path to dmcsd/dmcsc
export LOGPATH='.' # path to output logfiles

DORUN=no # run with `run'
VERBOSE=yes # output stuff
LOGDAEMONS=yes # log daemon output

# test instance
export TOPO=ring
declare -i CTX=3
declare -i SIG=10
declare -i BRS=5
declare -i RLS=5
export INST=a

export MODE=streaming
export PACK=200 # report at most $PACK many equilibria

################### below we don't need to touch anything

export TESTNAME=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST
declare -i MINPORT=5000

export DMCSD=$DMCSPATH/dmcsd
export DMCSC=$DMCSPATH/dmcsc

declare -i N
for (( N = 1 ; N <= $CTX ; N++ )); do

	INPUT=$TESTSPATH/$TESTNAME
	INPUTN=$TESTSPATH/$TESTNAME-$N

	LOGN=/dev/null
	RUNLOGN=/dev/null
	if [ x$LOGDAEMONS = xyes ] ; then
		LOGN=$LOGPATH/$TESTNAME-$MODE-$N.log
		RUNLOGN=$LOGPATH/$TESTNAME-$MODE-run-$N.log
	fi

	DMCSDOPTS="--context=$N --port=$((MINPORT+N)) --kb=$INPUTN.lp --br=$INPUTN.br --topology=$INPUT.opt"

	if [ x$VERBOSE = xyes ] ; then
	    #echo Starting context $N on port $((MINPORT+N)) with test $INPUTN.lp and $INPUTN.br and $INPUT.opt, log=$LOGN
	    set -x
	fi

	if [ x$DORUN = xyes ] ; then
	    $RUN $RUNLOGN $DMCSD $DMCSDOPTS > $LOGN 2>&1 &
	else
	    $DMCSD $DMCSDOPTS > $LOGN 2>&1 &
	fi

	set +x

	disown $! # don't record jobid, so we don't report kill messages

done

sleep 5

LOGTIME=$LOGPATH/$TESTNAME-$MODE-time.log
LOGCOUT=$LOGPATH/$TESTNAME-$MODE.log
LOGCERR=$LOGPATH/$TESTNAME-$MODE-err.log
LOGRUN=$LOGPATH/$TESTNAME-$MODE-run.log

DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1)) --system-size=$CTX"
MODEOPTS="--s=0"
if [ x$MODE = xstreaming ] ; then
    MODEOPTS="--s=1 --k=$PACK"
fi

if [ x$VERBOSE = xyes ] ; then
    set -x
fi

if [ x$DORUN = xyes ] ; then
    $GNUTIME $LOGTIME $RUN $LOGRUN $DMCSC $DMCSCOPTS $MODEOPTS > $LOGCOUT 2> $LOGCERR
else
    $GNUTIME $LOGTIME $DMCSC $DMCSCOPTS $MODEOPTS > $LOGCOUT 2> $LOGCERR
fi

set +x

# just to be safe
killall -1 dmcsd # 2>/dev/null
killall -9 dmcsd 2>/dev/null
