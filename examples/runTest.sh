#!/bin/bash

# disable malloc check in glibc otw. we get corruption blarg on exit() with NDEBUG builds
export MALLOC_CHECK_=0

declare -i TIMEOUT=200
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


declare -a sizes=(7 7 7 7 100 100 7 7 100 100 7 7 100 100)

declare -a topoNum=(3 4 6 1)

declare -a start=(0 2 6 10)
declare -a length=(2 4 4 4)

declare -a sigs=(10 40 40 100)
declare -a bridges=(5 20 20 50)
declare -a rels=(5 20 20 50)

declare -a packages=(10 100 0)

declare -i i=0
declare -i j=0
declare -i k=0
declare -i p=0
declare -i currentTopoNum=0


cd experiments

for CTOPO in tree ring zig-zag ; do 
#for CTOPO in diamond ; do 
    
    declare -i currentTopoNum=${topoNum[$i]}

    cd $CTOPO

    declare -i start_now=${start[$i]}

    declare -i end_now=$start_now
    let "end_now += ${length[$i]}"

    for (( j=$start_now; j < $end_now; ++j )); do
	export TOPO=$CTOPO	
	declare -i CTX=${sizes[$j]}

	for (( k = 0; k < 2; ++k )); do
#	for (( k = 0; k < 1; ++k )); do

	    declare -i l=${k}
	    let "l *= 2"

	    if [ $CTX = 100 ] ; then
		let "l += 1"
	    fi

	    declare -i SIG=${sigs[$l]}
	    declare -i BRS=${bridges[$l]}
	    declare -i RLS=${rels[$l]}

	    cd $CTX-$SIG-$BRS-$RLS
	    
	    for x in {a..e} ; do
#	    for x in {a..j} ; do
		export INST=$x
		
		TEMPLATE=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST
		
		cd $TEMPLATE

		declare -i MINPORT=5000

		export DMCSD="../../../../$DMCSPATH/dmcsd"
		export DMCSC="../../../../$DMCSPATH/dmcsc"

		export TESTNAME=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST
		declare -i MINPORT=5000

################ OPT #########################################

		export MODE=opt
		$GNUTIME /dev/null ls > /dev/null 2>&1
		if [ $? != 0 ]; then
		    echo I need GNU time, please setup GNUTIME properly. Bailing out.
		    exit 1
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
		    
		    pwd

		    $GNUTIME $TIMELOGN $DMCSDRUN > $LOGN 2>&1 &
		    
		    set +x
		    
		    disown $! # don't record jobid, so we don't report kill messages

		done

		sleep 50

		LOGTIME=./$TESTNAME-$MODE-time.log
		LOGCOUT=./$TESTNAME-$MODE.log
		LOGCERR=./$TESTNAME-$MODE-err.log
		LOGRUN=./$TESTNAME-$MODE-run.log
		
		DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1)) --system-size=$CTX --s=0"
		
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
		    echo "New test $TEMPLATE-$MODE finished: **$RESULT**" | mail -s "New test FINISHED" daotranminh@gmail.com
		else
		    egrep -H "status" $LOGPATH/$TESTNAME-$MODE-run*.log >> $STATSLOG
		    echo FAILED: $TESTNAME-$MODE >> $STATSLOG
		    echo "New test $TEMPLATE-$MODE " | mail -s "New test FAILED" daotranminh@gmail.com
		fi
		
		echo >> $STATSLOG
		
		date >> $STATSLOG
		
		cat $STATSLOG
		

################ STREAMING ###################################

		export MODE=streaming

		for (( p=0; p < 3; ++p )); do
		    declare -i PACK=${packages[$p]}
		    
		    STATSLOG=./$TESTNAME-$MODE-k$PACK-status.log
		    date > $STATSLOG

		    declare -i N
		    for (( N = 1 ; N <= $CTX ; N++ )); do
			
			INPUT=./$TESTNAME
			INPUTN=./$TESTNAME-$N
			
			LOGN=/dev/null
			RUNLOGN=/dev/null
			TIMELOGN=/dev/null
			
			if [ x$LOGDAEMONS = xyes ] ; then
			    LOGN=./$TESTNAME-$MODE-k$PACK-$N.log
			    RUNLOGN=./$TESTNAME-$MODE-k$PACK-run-$N.log
			    TIMELOGN=./$TESTNAME-$MODE-k$PACK-time-$N.log
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
			
			pwd
			
			$GNUTIME $TIMELOGN $DMCSDRUN > $LOGN 2>&1 &
			
			set +x
			
			disown $! # don't record jobid, so we don't report kill messages

		    done

		    sleep 50
		    
		    LOGTIME=./$TESTNAME-$MODE-k$PACK-time.log
		    LOGCOUT=./$TESTNAME-$MODE-k$PACK.log
		    LOGCERR=./$TESTNAME-$MODE-k$PACK-err.log
		    LOGRUN=./$TESTNAME-$MODE-k$PACK-run.log
		    
		    DMCSCOPTS="--hostname=localhost --port=$((MINPORT+1)) --system-size=$CTX --s=1 --k=$PACK"
		    
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
			echo "New test $TEMPLATE-$MODE-k$PACK finished: **$RESULT**" | mail -s "New test FINISHED" daotranminh@gmail.com
		    else
			egrep -H "status" $LOGPATH/$TESTNAME-$MODE-run*.log >> $STATSLOG
			echo FAILED: $TESTNAME-$MODE >> $STATSLOG
			echo "New test $TEMPLATE-$MODE-k$PACK " | mail -s "New test FAILED" daotranminh@gmail.com
		    fi
		    
		    echo >> $STATSLOG
		    
		    date >> $STATSLOG
		
		    cat $STATSLOG

		done
##############################################################
		cd ..
	    done
	    cd ..	    
	done
    done
    cd ..
    let "i += 1"
done
