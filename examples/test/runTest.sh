#!/bin/bash

DOECHO=yes
LOGDAEMOND=yes

testpack[0]="all"
testpack[1]="10"

#################################################################################
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

#################################################################################
moveLogFiles()
{
    no_args=$#
    optimized=opt_
    if [ $no_args -eq 3 ] ; then
	optimized=
    fi
    
    toponame=$1
    testname=$2
    mode=$optimized$3
    basedir=../../..

    createSubDir $basedir/output
    createSubDir $basedir/output/$toponame
    createSubDir $basedir/output/$toponame/$testname
    createSubDir $basedir/output/$toponame/$testname/$mode

    rm $basedir/output/$toponame/$testname/$mode/*
    mv *.log $basedir/output/$toponame/$testname/$mode
} # end of moveLogFiles

#################################################################################
for topofile in `ls config/*.cfg` ; do
    length=${#topofile}
    let "length -=11"
    toponame=${topofile:7:$length}

    wantlog=
    if [ x$LOGDAEMOND = xyes ] ; then
	wantlog=-log
    fi

    for line in `cat $topofile` ; do
	list=(`echo $line | tr ',' ' ' `)
	testname=$toponame-${list[1]}-${list[2]}-${list[3]}-${list[4]}-${list[5]}

	cd data/$toponame/$testname
	pwd

	no_test_packs=${#testpack[@]}
	for ((i = 0; i < $no_test_packs; ++i)) ; do
	    bash $toponame-command-line-${testpack[$i]}$wantlog.sh
	    moveLogFiles $toponame $testname ${testpack[$i]}

	    bash $toponame-command-line-opt-${testpack[$i]}$wantlog.sh
	    moveLogFiles $toponame $testname ${testpack[$i]} opt
	done

	cd ../../.. # get out of data/$toponame/$testname
    done
done