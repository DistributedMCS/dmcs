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
export DMCS_GEN_PATH='../build/src/generator' # path to dmcsGen
#export LOGPATH='./experiments' # path to output logfiles (should be fixed)

DORUN=yes # run with `run'
DOTIMELIMIT=yes # run with `timelimit'
VERBOSE=yes # output stuff
LOGDAEMONS=yes # log daemon output

TESTCASES=testcases.txt

#                   D T R Z
declare -a topoNum=(1 6 4 3)

#                 D        T               R               Z
declare -a sizes=(10 28 34 10 28 34 50 100 10 28 34 50 100 10 28 34 50 100)

declare -a start=(0 3 8 13 18)
declare -a length=(3 5 5 5)

declare -a sigs=(10 40)
declare -a bridges=(5 20)
declare -a rels=(5 20)

declare -i i=0
declare -i j=0
declare -i k=0
declare -i currentTopoNum=0

rm $TESTCASES

cd experiments

DMCS_EXPR_DIR=$?

if [ DMCS_EXPR_DIR != 0 ] ; then
    mkdir experiments
    cd experiments
fi

for TOPO in diamond tree ring zig-zag ; do 
    
    declare -i currentTopoNum=${topoNum[$i]}

    mkdir $TOPO
    cd $TOPO

    declare -i start_now=${start[$i]}

    declare -i end_now=$start_now
    let "end_now += ${length[$i]}"

    for (( j=$start_now; j < $end_now; ++j )); do
	declare -i CTX=${sizes[$j]}

	for (( k = 0; k < 2; ++k )); do

	    declare -i SIG=${sigs[$k]}
	    declare -i BRS=${bridges[$k]}
	    declare -i RLS=${rels[$k]}

	    mkdir $CTX-$SIG-$BRS-$RLS
	    cd $CTX-$SIG-$BRS-$RLS
	    
	    for x in {a..j} ; do
		export INST=$x
		
		TEMPLATE=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST

		echo $TEMPLATE >> ../../../$TESTCASES
		
		mkdir $TEMPLATE 
		cd $TEMPLATE
		
		export DMCSGEN=$DMCS_GEN_PATH/dmcsGen

		pwd
		
		DMCSGENS="--context=$CTX --atoms=$SIG --interface=$BRS --bridge_rules=$RLS --topology=$currentTopoNum --prefix=$TEMPLATE"
		DMCSGENRUN="../../../../$DMCSGEN $DMCSGENS"
		
		$DMCSGENRUN
		cd ..
	    done

	    echo "end" >> ../../../$TESTCASES
	    cd ..	    
	done
    done
    cd ..
    let "i += 1"
done