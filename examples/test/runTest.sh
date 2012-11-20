#!/bin/bash

DOECHO=yes
WANTLOG=yes
EMAIL=dao@kr.tuwien.ac.at

testpack[0]="all"
testpack[1]="1"
testpack[2]="10"
testpack[3]="100"

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

    if [ x$WANTLOG = xyes ] ; then
	rm $basedir/output/$toponame/$testname/$mode/*
    fi

    mv *.log $basedir/output/$toponame/$testname/$mode
} # end of moveLogFiles

#################################################################################
runOneInstance()
{
    filename=$1
    toName=$2
    teName=$3
    tpack=$4
    runOpt=$5

    basedir=../../..

    wantlog=
    if [ x$WANTLOG = xyes ] ; then
	wantlog=-log
    fi

    wantOpt=
    if [ x$runOpt = xopt ] ; then
	wantOpt=-opt
    fi

    echo "Run "$teName $tpack $runOpt
    bash $toName-command-line$wantOpt-$tpack$wantlog.sh
    RETVAL=$?
    echo $RETVAL

    if [ $RETVAL = 0 ] ; then
	echo "PASSED: $teName $tpack $runOpt" > $toName-status.log
	echo $toName,$teName,$tpack,$runOpt >> $basedir/passedtests.log
	(cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "PASSED: dmcs testcase on GLUCK: $teName $tpack $runOpt" $EMAIL
    else
	echo "FAILED: $teName $tpack $runOpt" > $toName-status.log
	echo $toName,$teName,$tpack,$runOpt >> $basedir/$filename
	(cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "FAILED: dmcs testcase on GLUCK: $teName $tpack $runOpt" $EMAIL
    fi

    moveLogFiles $toName $teName $tpack $runOpt
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
	    
	    no_test_packs=${#testpack[@]}
	    for ((i = 0; i < $no_test_packs; ++i)) ; do
		runOneInstance $filename $toponame $testname ${testpack[$i]}
		runOneInstance $filename $toponame $testname ${testpack[$i]} opt
	    done
	    
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
	# list[2] = testpack
	# list[3] = (mode)
	cd data/${list[0]}/${list[1]}
	if [ $noArguments -eq 4 ] ; then
	    runOneInstance failedtests.tmp ${list[0]} ${list[1]} ${list[2]} ${list[3]}
	else
	    runOneInstance failedtests.tmp ${list[0]} ${list[1]} ${list[2]}
	fi
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
