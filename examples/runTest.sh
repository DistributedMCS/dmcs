#!/bin/bash

# disable malloc check in glibc otw. we get corruption blarg on exit() with NDEBUG builds
export MALLOC_CHECK_=0

declare -i TIMEOUT=100
declare -i MEMOUT=1000

export GNUTIME="/usr/bin/time --verbose -o" # time command
export RUN="run -s $MEMOUT -t $((TIMEOUT+20)) -k -o"
export TIMELIMIT="timelimit -p -s 1 -t $TIMEOUT -T 20"
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S' # time format
#export TESTSPATH='./experiments' # path to lp/br/opt
export DMCSPATH='../build/src' # path to dmcsd/dmcsc
#export LOGPATH='./experiments' # path to output logfiles (should be fixed)

DORUN=yes # run with `run'
DOTIMELIMIT=yes # run with `timelimit'
VERBOSE=yes # output stuff
LOGDAEMONS=yes # log daemon output


declare -a sizes=(7 7 100 7 100 7 100)

declare -a start=(0 1 3 5)
declare -a length=(1 2 2 2)

declare -a sigs=(10 40 40 100)
declare -a bridges=(5 20 20 50)
declare -a rels=(5 20 20 50)

declare -a packages=(10 100 0)

declare -i i=0
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

    if [ $MODE = streaming ] ; then
	MODE=$MODE-k$2
    fi

    echo $TESTNAME-$MODE
    echo "Sleeping for 70 secs..."
    sleep 70

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
	    LOGN=./$TESTNAME-$MODE-$N.log
	    RUNLOGN=./$TESTNAME-$MODE-run-$N.log
	    TIMELOGN=./$TESTNAME-$MODE-time-$N.log
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
    
    echo "Sleep for 40 secs for the Server to start..."
    sleep 40
    
    LOGTIME=./$TESTNAME-$MODE-time.log
    LOGCOUT=./$TESTNAME-$MODE.log
    LOGCERR=./$TESTNAME-$MODE-err.log
    LOGRUN=./$TESTNAME-$MODE-run.log

    if [ $1 = streaming ] ; then 
	DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1)) --system-size=$CTX --s=1 --k=$2"
    else
	DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1)) --system-size=$CTX --s=0"
    fi
		
    DMCSCRUN="$DMCSC $DMCSCOPTS "
    
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
    
    killall -1 dmcsd # 2>/dev/null
    killall -9 dmcsd 2>/dev/null
    
    echo >> $STATSLOG
    
    if [ $DMCSCRET = 0 ]; then
	echo PASS: $TESTNAME-$MODE >> $STATSLOG
	echo "cat $TESTNAME-$MODE.log | head -1" >> $RESULT
	echo "New test $TEMPLATE-$MODE finished: **$RESULT**" | mail -s "New test FINISHED" dao@kr.tuwien.ac.at
    else
	egrep -H "status" $LOGPATH/$TESTNAME-$MODE-run*.log >> $STATSLOG
	echo FAILED: $TESTNAME-$MODE >> $STATSLOG
	echo "New test $TEMPLATE-$MODE " | mail -s "New test FAILED" dao@kr.tuwien.ac.at
    fi
    
    echo >> $STATSLOG
    
    date >> $STATSLOG
    
    cat $STATSLOG
}

##########################################################################

########## check GNUTIME ##########

#$GNUTIME /dev/null ls > /dev/null 2>&1
#if [ $? != 0 ]; then
#    echo I need GNU time, please setup GNUTIME properly. Bailing out.
#    exit 1
#fi

cd experiments

for TOPO in diamond tree ring zig-zag ; do 
    
    cd $TOPO
    #export TOPO=$CTOPO

    declare -i start_now=${start[$i]}

    declare -i end_now=$start_now
    let "end_now += ${length[$i]}"

    # go for system size ########################
    for (( j=$start_now; j < $end_now; ++j )); do 

	declare -i CTX=${sizes[$j]}

	# go for parameters setting #############
	for (( k = 0; k < 2; ++k )); do

	    declare -i l=${k}
	    let "l *= 2"

	    if [ $CTX = 100 ] ; then
		let "l += 1"
	    fi

	    declare -i SIG=${sigs[$l]}
	    declare -i BRS=${bridges[$l]}
	    declare -i RLS=${rels[$l]}

	    cd $CTX-$SIG-$BRS-$RLS
	    
	    # go for instances #########################
	    for x in {a..e} ; do
		export INST=$x
		
		TEMPLATE=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST
		
		cd $TEMPLATE

		declare -i MINPORT=5000

		export DMCSD="../../../../$DMCSPATH/dmcsd"
		export DMCSC="../../../../$DMCSPATH/dmcsc"
		export TESTNAME=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST
		declare -i MINPORT=5000

		runTest opt

		for (( p=0; p < 3; ++p )); do
		    runTest streaming ${packages[$p]}
		done
		cd ..
	    done # for x in {a..j}
	    cd ..	    
	done # for k = 0...
    done # for j = $start_now...$end_now
    cd ..
    let "i += 1"
done # for TOPO
