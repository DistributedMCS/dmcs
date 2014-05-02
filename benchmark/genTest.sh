#!/bin/bash

DMCSPATH=../../../../../build/src
DMCSGEN=$DMCSPATH/generator/new_dmcsGen
DOECHO=yes
TIMEOUT=600

###############################################################################

createSubDir()
{
    dirName=$1
    
    if [ ! -e $dirName ] || [ ! -d $dirName ] ; then
	mkdir $dirName
	if [ x$DOECHO = xyes ] ; then
	    echo Create subdir $dirName
	fi
    fi

} # end createSubDir

###############################################################################

createOneTest()
{
    gdt=$1
    ps=$2
    tname=$3
    lkbt=$4
    args=("${!5}")

    # args[0] = topo_id
    # args[1] = system_size
    # args[2] = no_atoms
    # args[3] = no_interface
    # args[4] = no_bridgerules
    # args[5] = instances
    
    testname=$tname-${args[1]}-${args[2]}-${args[3]}-${args[4]}-${args[5]}

    createSubDir $testname

    cd $testname

    RUNGEN="$DMCSGEN --topology=${args[0]} --contexts=${args[1]} --atoms=${args[2]} --interface=${args[3]} --bridge_rules=${args[4]} --packsize=$ps --prefix=$tname --dmcspath=$DMCSPATH --timeout=$TIMEOUT"

    if [ x$gdt = xyes ] ; then
	RUNGEN="$DMCSGEN --gen-data=true --local-kb-type=$lkbt --topology=${args[0]} --contexts=${args[1]} --atoms=${args[2]} --interface=${args[3]} --bridge_rules=${args[4]} --packsize=$ps --prefix=$tname --dmcspath=$DMCSPATH --timeout=$TIMEOUT"
    fi

    if [ x$DOECHO = xyes ] ; then
	echo "Calling $RUNGEN"
    fi

    $RUNGEN

    cd .. # get out of $testname

} # end createOneTest


###############################################################################
#
# extract command line arguments:
#
# -d yes/no -p N -n DIRECTORY -lkb 0/1
#
# -d: generate data (yes) or just command lines (no)
# -p: pack size
# -n: name of subdirectory for storing data
# -lkb: local knowledge base type

gendata=no
packsize=0
subdir=default
localKbType=0 # 0: non-deterministic, 1: deterministic

while [ $# -gt 0 ]
do
    case "$1" in
	-h) echo >&2 "Usage: $0 [-d yes/no] [-p N] [-n DIRECTORY] [-lkb 0/1]"
	    exit 0;;
	-d) gendata="$2"; shift;;
	-p) packsize="$2"; shift;;
	-n) subdir="$2"; shift;;
	-lkb) localKbType="$2"; shift;;
	-*) echo >&2 "Usage: $0 [-d yes/no] [-p N] [-n DIRECTORY] [-lkb 0/1]"
	    exit 1;;
	*) break;;
    esac
    shift

    if [ x$packsize = x ] ; then
	packsize=0
    fi
done

createSubDir $subdir
cd $subdir

createSubDir config
createSubDir data

python ../listTests.py --ext=cfg

for topofile in `ls config/*.cfg` ; do
    length=${#topofile}
    let "length -= 11"
    toponame=${topofile:7:$length}

    for line in `cat $topofile` ; do
	list=(`echo $line | tr ',' ' ' `)
	no_arguments=${#list[@]}

	if [ $no_arguments -ne 6 ] ; then
	    echo "Bailing out: Number of arguments should be 6!"
	    exit 1
	fi

	cd data
	createSubDir $toponame

	cd $toponame

	if [ x$gendata = xyes ] ; then
	    if [ x$DOECHO = xyes ] ; then
		echo Sleep 10 secs...
	    fi
	    sleep 10
	fi

	createOneTest $gendata $packsize $toponame $localKbType list[@]

	cd ../.. # get out of data/$toponame
    done
done

cd .. # get out of $subdir
