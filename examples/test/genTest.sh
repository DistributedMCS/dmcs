#!/bin/bash

DMCSGEN=../../../../build/src/generator/new_dmcsGen

createOneTest()
{
    gdt=$1
    ps=$2
    tname=$3
    args=("${!4}")

    # args[0] = topo_id
    # args[1] = system_size
    # args[2] = no_atoms
    # args[3] = no_interface
    # args[4] = no_bridgerules
    
    testname=$tname-${args[1]}-${args[2]}-${args[3]}-${args[4]}

    if [ ! -e $testname ] || [ ! -d $testname ] ; then
	echo Create subdir $testname
	mkdir $testname
    fi

    cd $testname

    RUNGEN="$DMCSGEN --topology=${args[0]} --contexts=${args[1]} --atoms=${args[2]} --interface=${args[3]} --bridge_rules=${args[4]} --packsize=$ps --prefix=$tname"

    if [ x$gdt = xyes ] ; then
	RUNGEN="$DMCSGEN --gen-data=true --topology=${args[0]} --contexts=${args[1]} --atoms=${args[2]} --interface=${args[3]} --bridge_rules=${args[4]} --packsize=$ps --prefix=$tname"
    fi

    echo "Calling $RUNGEN"
    $RUNGEN

    cd .. # get out of $testname

} # end createOneTest

###############################################################################

# extract command line arguments:
#
# -d yes/no -p N
#

gendata=no
packsize=0

while [ $# -gt 0 ]
do
    case "$1" in
	-h) echo >&2 "Usage: $0 [-d yes/no] [-p N]"
	    exit 0;;
	-d) gendata="$2"; shift;;
	-p) packsize="$2"; shift;;
	-*) echo >&2 "Usage: $0 [-d yes/no] [-p N]"
	    exit 1;;
	*) break;;
    esac
    shift

    if [ x$packsize = x ] ; then
	packsize=0
    fi
done

python listTests.py

for topofile in `ls *.cfg` ; do
    length=${#topofile}
    let "length -= 4"
    toponame=${topofile:0:$length}

    for line in `cat $topofile` ; do
	list=(`echo $line | tr ',' ' ' `)
	no_arguments=${#list[@]}

	if [ $no_arguments -ne 5 ] ; then
	    echo "Bailing out: Number of arguments in config file should be 5!"
	    exit 1
	fi

	if [ ! -e $toponame ] || [ ! -d $toponame ] ; then
	    echo Creating subdir $toponame
	    mkdir $toponame
	fi

	cd $toponame
	createOneTest $gendata $packsize $toponame list[@]
	cd .. # get out of $toponame
    done
done
