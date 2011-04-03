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


#each topology has 3 sizes that we test
declare -a sizes=(7 7 7 7 100 100 7 7 100 100 7 7 100 100)

declare -a topoNum=(1 6 4 3)

declare -a start=(0 2 6 10)
declare -a length=(2 4 4 4)

declare -a sigs=(10 40 40 100)
declare -a bridges=(5 20 20 50)
declare -a rels=(5 20 20 50)

declare -i i=0
declare -i j=0
declare -i k=0
declare -i currentTopoNum=0

#for each topo
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

	    declare -i l=${k}
	    let "l *= 2"

	    if [ $CTX = 100 ] ; then
		let "l += 1"
	    fi

	    declare -i SIG=${sigs[$l]}
	    declare -i BRS=${bridges[$l]}
	    declare -i RLS=${rels[$l]}

	    mkdir $CTX-$SIG-$BRS-$RLS
	    cd $CTX-$SIG-$BRS-$RLS
	    
	    for x in {a..j} ; do
		export INST=$x
		
		TEMPLATE=$TOPO-$CTX-$SIG-$BRS-$RLS-$INST
		
		mkdir $TEMPLATE 
		cd $TEMPLATE
		
		export DMCSGEN=$DMCS_GEN_PATH/dmcsGen

		pwd
		
		DMCSGENS="--context=$CTX --atoms=$SIG --interface=$BRS --bridge_rules=$RLS --topology=$currentTopoNum --prefix=$TEMPLATE"
		DMCSGENRUN="../../../../$DMCSGEN $DMCSGENS"
		
		$DMCSGENRUN
		cd ..
	    done
	    cd ..	    
	done
    done
    cd ..
    let "i += 1"
done