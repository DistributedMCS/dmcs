#!/bin/bash

python listTests.py

for topofile in `ls *.cfg` ; do
    length=${#topofile}
    let "length -= 4"
    toponame=${topofile:0:$length}

    for line in `cat $topofile` ; do
	list=(`echo $line | tr ',' ' ' `)
	no_arguments=${#list[@]}

	if [ $no_arguments -ne 5 ] ; then
	    echo ERROR
	    exit 1
	fi

	#TODO: create subdir if not exists. clean subdir if existed.
	#call new_dmcsGen to create data files
	#another part for just creating commandline files
    done
done
