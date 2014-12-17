#!/bin/bash

export TIMEOUT=600

DOECHO=yes
WANTLOG=no
MAXTRY=2
EMAIL=dao@kr.tuwien.ac.at

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
    mvTopoName=$1
    mvTestName=$2

    basedir=../../..

    createSubDir $basedir/output-mcsie
    createSubDir $basedir/output-mcsie/$mvTopoName
    createSubDir $basedir/output-mcsie/$mvTopoName/$mvTestName

    if [ x$WANTLOG = xyes ] ; then
	rm $basedir/output-mcsie/$mvTopoName/$mvTestName/*
    fi
    
    mv *.log $basedir/output-mcsie/$mvTopoName/$mvTestName
} # end of moveLogFiles

#################################################################################
runOneInstance()
{
    filename=$1
    toName=$2
    teName=$3

    basedir=../../..

    declare -i j
    for (( j=1; j <= $MAXTRY; ++j)) ; do
	if [ $j -eq 1 ] ; then
	    echo "Run "$teName 
	else
	    echo "Retry($j) "$teName
	fi
	
	/usr/bin/time --verbose -o $toName-time.log /usr/bin/timeout -k 20 $TIMEOUT dlvhex2 --plugindir=\!:/home/dao/sources/dlvhex/mcsieplugin/build/src --ieenable --iemode=eq --iepath=./ master.hex > $toName.log 2> $toName-err.log

	RETVAL=$?
	echo $RETVAL
	
	if [ $RETVAL -eq 0 ] ; then
	    echo "PASSED: $teName" > $toName-status.log
	    echo $toName,$teName >> $basedir/passedtests.log
	    (cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "PASSED: dmcs-mcsie testcase on GLUCK: $teName" $EMAIL
	    break
	elif [ $RETVAL -eq 124 ] ; then
	    echo "TIMEOUT: $teName" > $toName-status.log
	    echo $toName,$teName >> $basedir/timeouttests.log
	    (cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "TIMEOUT: dmcs-mcsie testcase on GLUCK: $teName" $EMAIL
	    break
	else
	    if [ $j -eq $MAXTRY ] ; then
		echo "FAILED: $teName" > $toName-status.log
		echo $toName,$teName >> $basedir/$filename
		(cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "FAILED: dmcs-mcsie testcase on GLUCK: $teName" $EMAIL
	    else
		(cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "FAILED($j): dmcs testcase on GLUCK: $teName" $EMAIL
	    fi
	fi
    done

    moveLogFiles $toName $teName
} # end of runOneInstance

#################################################################################
runAll()
{
    filename=$1
    for topofile in `ls config/*.cfg` ; do
	length=${#topofile}
	let "length -=11"
	toponame=${topofile:7:$length}
	
	for line in `cat $topofile` ; do
	    list=(`echo $line | tr ',' ' ' `)
	    testname=$toponame-${list[1]}-${list[2]}-${list[3]}-${list[4]}-${list[5]}
	    
	    cd data/$toponame/$testname
	    pwd
	    
	    runOneInstance $filename $toponame $testname 
	    
	    cd ../../.. # get out of data/$toponame/$testname
	done
    done
} # end of runAll

#################################################################################
reRun()
{
    filename=$1
    for line in `cat $filename` ; do
	echo $line
	list=(`echo $line | tr ',' ' ' `)
	noArguments=${#list[@]}

	# list[0] = toponame
	# list[1] = testname
	cd data/${list[0]}/${list[1]}
	
	runOneInstance failedtests.tmp ${list[0]} ${list[1]} ${list[2]}

	cd ../../.. # get out of data/$toponame/$testname
    done
} # end of reRun()

#################################################################################
#
# extract command line arguments:
#
# -f FILE -r
#
# -f: filename to store FAILED test cases
# -r: rerun all FAILED testcases

filename=failedtests.log
rerun=no

while [ $# -gt 0 ]
do
    case "$1" in
	-h) echo >&2 "Usage: $0 [-f FILE] [-r]"
	    exit 0;;
	-f) filename="$2"; shift;;
	-r) rerun=yes;; 
	-*) echo >&2  "Usage: $0 [-f FILE] [-r]"
	    exit 1;;
	*) break;;
    esac
    shift
done

if [ x$rerun = xyes ] ; then
    reRun $filename
else
    runAll $filename
fi
