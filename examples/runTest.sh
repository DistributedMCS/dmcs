#!/bin/bash

# disable malloc check in glibc otw. we get corruption blarg on exit() with NDEBUG builds
export MALLOC_CHECK_=0

declare -i TIMEOUT=180
declare -i MEMOUT=1000

export GNUTIME="/usr/bin/time --verbose -o" # time command
export RUN="run -s $MEMOUT -t $((TIMEOUT+20)) -k -o"
export TIMELIMIT="timelimit -p -s 1 -t $TIMEOUT -T 20"
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S' # time format
export TESTSPATH='./experiments' # path to lp/br/opt
export DMCSPATH='../../../../../build/src' # path to dmcsd/dmcsc relative to test instances
export DMCS_PARALLEL_PATH='../../../../../../dmcs-parallel/build/src' # path to parallel dmcsd/dmcsc relative to test instances
export LOGPATH='.' # path to output logfiles relative to test instances

export EMAIL=dao@kr.tuwien.ac.at

DORUN=no # run with `run'
DOTIMELIMIT=yes # run with `timelimit'
VERBOSE=yes # output stuff
LOGDAEMONS=no # log daemon output
ULIMIT=yes # use ulimit

export TOPOLOGIES="diamond tree ring zig-zag"
#export TOPOLOGIES="tree ring zig-zag"
export INSTANCES=a..e

#                 D        T               R               Z
declare -a sizes=(10 28 34 10 28 34 50 100 10 28 34 50 100 10 28 34 50 100)
declare -a start=(0 3 8 13 18)
declare -a length=(3 5 5 5)

declare -a sigs=(10 40)
declare -a bridges=(5 20)
declare -a rels=(5 20)

declare -a packages=(10 100 0)

################### below we don't need to touch anything

export DMCSD="$DMCSPATH/dmcsd"
export DMCSC="$DMCSPATH/dmcsc"

export DMCSD_PARALLEL="$DMCS_PARALLEL_PATH/dmcsd"
export DMCSC_PARALLEL="$DMCS_PARALLEL_PATH/dmcsc"


declare -i MINPORT=5000

declare -i i=1
declare -i j=0
declare -i k=0
declare -i p=0

######## run a test case ##################################################
#
# @param:
#
# $1: MODE: opt or streaming
# $2: PACK: when $1=streaming 
#
##########################################################################

function runTest()
{
    MODE=$1

    if [ $MODE = streaming -o $MODE = parallel ] ; then
	MODE=$MODE-k$2
    fi

    echo $TESTNAME-$MODE

    if [ `netstat -anlt | grep TIME_WAIT | wc -l` -gt 1 ]; then # kludge
	echo "Sleeping for 70 secs..."
	sleep 70 
    fi


    STATSLOG=./$TESTNAME-$MODE-status.log
    date > $STATSLOG

    declare -i N
    for (( N = 1 ; N <= $CTX ; N++ )); do
		    
	INPUT=./$TESTNAME
	INPUTN=./$TESTNAME-$N
		    
	LOGN=/dev/null
	RUNLOGN=/dev/null
	TIMELOGN=/dev/null

	if [ x$LOGDAEMONS = xyes ] ; then
	    LOGN=$LOGPATH/$TESTNAME-$MODE-$N.log
	    RUNLOGN=$LOGPATH/$TESTNAME-$MODE-run-$N.log
	    TIMELOGN=$LOGPATH/$TESTNAME-$MODE-time-$N.log
	fi

	DMCSDOPTS="--context=$N --port=$((MINPORT+N)) --kb=$INPUTN.lp --br=$INPUTN.br --topology=$INPUT.opt"

	if [ $1 = parallel ] ; then
	    DMCSDRUN="$DMCSD_PARALLEL $DMCSDOPTS"
	else
	    DMCSDRUN="$DMCSD $DMCSDOPTS"
	fi
	
	if [ x$DORUN = xyes ] ; then
	    DMCSDRUN="$RUN $RUNLOGN $DMCSDRUN"
	fi
	
	if [ x$DOTIMELIMIT = xyes ] ; then
	    DMCSDRUN="$TIMELIMIT $DMCSDRUN"
	fi

	if [ x$LOGDAEMONS = xyes ] ; then
	    DMCSDRUN="$GNUTIME $TIMELOGN $DMCSDRUN"
	fi
	
	if [ x$VERBOSE = xyes ] ; then
	    set -x
	fi

	if [ x$ULIMIT = xyes ] ; then
	    (ulimit -v $((MEMOUT*1024)) ; $DMCSDRUN ) > $LOGN 2>&1 &
	else
	    $DMCSDRUN > $LOGN 2>&1 &
	fi
	
	set +x
	
	disown $! # don't record jobid, so we don't report kill messages
	
    done
    
    declare -i SLEEPTIME=$((CTX * 4 / 10))
    echo "Sleep for $SLEEPTIME secs for the daemons to start..."
    sleep $SLEEPTIME
    
    LOGTIME=$LOGPATH/$TESTNAME-$MODE-time.log
    LOGCOUT=$LOGPATH/$TESTNAME-$MODE.log
    LOGCERR=$LOGPATH/$TESTNAME-$MODE-err.log
    LOGRUN=$LOGPATH/$TESTNAME-$MODE-run.log

    if [ $1 = streaming -o $1 = parallel ] ; then 
	DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1)) --system-size=$CTX --s=1 --k=$2"
    else
	DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1)) --system-size=$CTX --s=0"
    fi

    if [ $1 = parallel ] ; then
	DMCSCRUN="$DMCSC_PARALLEL $DMCSCOPTS "
    else
	DMCSCRUN="$DMCSC $DMCSCOPTS "
    fi
    
    if [ x$DORUN = xyes ] ; then
	DMCSCRUN="$RUN $LOGRUN $DMCSCRUN"
    fi
    
    if [ x$DOTIMELIMIT = xyes ] ; then
	DMCSCRUN="$TIMELIMIT $DMCSCRUN"
    fi

    DMCSCRUN="$GNUTIME $LOGTIME $DMCSCRUN"

    if [ x$VERBOSE = xyes ] ; then
	set -x
    fi

    if [ x$ULIMIT = xyes ] ; then
    	( ulimit -v $((MEMOUT*1024)) ; $DMCSCRUN ) > $LOGCOUT 2> $LOGCERR
    else
	$DMCSCRUN > $LOGCOUT 2> $LOGCERR
    fi
    
    DMCSCRET=$?
    
    set +x
    
    killall -1 dmcsd # 2>/dev/null
    killall -9 dmcsd 2>/dev/null
    
    echo >> $STATSLOG
    
    if [ $DMCSCRET = 0 ]; then
	echo "PASS: $TESTNAME-$MODE" >> $STATSLOG
	(echo "New test $TESTNAME-$MODE finished" ; echo ; cat $TESTNAME-$MODE.log ; echo ; cat $TESTNAME-$MODE-time.log) | mail -s "New test on lion FINISHED" $EMAIL
    else
	egrep -H "status" $LOGPATH/$TESTNAME-$MODE-run*.log >> $STATSLOG
	echo "FAILED: $TESTNAME-$MODE" >> $STATSLOG
	(echo "New test $TESTNAME-$MODE failed" ; echo ; cat $TESTNAME-$MODE.log ; echo ; cat $TESTNAME-$MODE-time.log) | mail -s "New test on lion FAILED" $EMAIL
    fi
    
    echo >> $STATSLOG
    
    date >> $STATSLOG
    
    cat $STATSLOG
}

##########################################################################

########## check GNUTIME ##########

$GNUTIME /dev/null ls > /dev/null 2>&1
if [ $? != 0 ]; then
    echo I need GNU time, please setup GNUTIME properly. Bailing out.
    exit 1
fi

cd $TESTSPATH

for TOPO in $TOPOLOGIES ; do 
    
    cd $TOPO

    declare -i start_now=${start[$i]}

    declare -i end_now=$start_now
    #let "end_now += 1"
    let "end_now += ${length[$i]}"

    # go for system size ########################
    for (( j=$start_now; j < $end_now; ++j )); do 

	declare -i CTX=${sizes[$j]}

	# go for parameters setting #############
	for (( k = 0; k < 2; ++k )); do
	#for (( k = 0; k < 1; ++k )); do

	    declare -i SIG=${sigs[$k]}
	    declare -i BRS=${bridges[$k]}
	    declare -i RLS=${rels[$k]}

	    cd $CTX-$SIG-$BRS-$RLS
	    
	    # go for instances #########################
	    for INST in $(eval echo {$INSTANCES}) ; do
		
		export TESTNAME=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST
		
		cd $TESTNAME

		runTest opt

		for (( p=0; p < 3; ++p )); do
		    runTest streaming ${packages[$p]}

		    runTest parallel ${packages[$p]}
		done

		cd ..
	    done # for INST
	    cd ..	    
	done # for k = 0...
    done # for j = $start_now...$end_now
    cd ..
    let "i += 1"
done # for TOPO
