#!/bin/bash

LOGFILENAME=checkQuery.log
CONTEXT=0
ATOM=c0a1

#################################################################################
checkTopo()
{
    topo=$1
    echo $topo
    topo=${topo:0:${#topo}-1}

    for instance in `ls -d */` ; do
	instance=${instance:0:${#instance}-1}
	python ../../checkQuery.py --instance=$instance --file=$instance/opt_10/$topo.log --logfile=../../$LOGFILENAME --context=$CONTEXT --atom=$ATOM
    done
}

rm $LOGFILENAME

cd output

for toponame in `ls -d */` ; do
    cd $toponame
    checkTopo $toponame
    cd ..
done

cd ..