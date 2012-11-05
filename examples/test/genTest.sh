#!/bin/bash

DMCSGEN=../../../../build/src/generator/new_dmcsGen

createOneTest()
{
    tname=$1
    args=("${!2}")

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

    RUNGEN="$DMCSGEN --topology=${args[0]} --contexts=${args[1]} --atoms=${args[2]} --interface=${args[3]} --bridge_rules=${args[4]} --prefix=$tname"
    pwd
    echo "Calling $RUNGEN"
    $RUNGEN

    cd .. # get out of $testname

} # end createOneTest

###############################################################################

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
	createOneTest $toponame list[@]
	cd .. # get out of $toponame
    done
done
