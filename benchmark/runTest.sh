#!/bin/bash

DOECHO=yes
WANTLOG=no
MAXTRY=4
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
    mvTopoName=$1
    mvTestName=$2
    mvTestPack=$3
    mvRunOpt=$4
    mvNoLoop=$5

    optimized=
    loopMode=
    mode=

    if [ x$mvRunOpt = xopt ] ; then
	optimized=opt_
    fi

    if [ x$mvNoLoop = xnoloop ] ; then
	loopMode=noloop_
    fi

    mode=$optimized$loopMode$3
    basedir=../../..

    createSubDir $basedir/output
    createSubDir $basedir/output/$mvTopoName
    createSubDir $basedir/output/$mvTopoName/$mvTestName
    createSubDir $basedir/output/$mvTopoName/$mvTestName/$mode

    if [ x$WANTLOG = xyes ] ; then
	rm $basedir/output/$mvTopoName/$mvTestName/$mode/*
    fi

    mv *.log $basedir/output/$mvTopoName/$mvTestName/$mode
} # end of moveLogFiles

#################################################################################
runOneInstance()
{
    passedTestFile=$1
    failedTestFile=$2
    timeoutTestFile=$3
    toName=$4
    teName=$5
    tpack=$6
    runOpt=$7
    noLoop=$8

    basedir=../../..

    wantlog=
    if [ x$WANTLOG = xyes ] ; then
	wantlog=-log
    fi

    wantOpt=
    if [ x$runOpt = xopt ] ; then
	wantOpt=-opt
    fi

    wantNoLoop=
    if [ x$noLoop = xnoloop ] ; then
	wantNoLoop=-noloop
    fi

    declare -i j
    for (( j=1; j <= $MAXTRY; ++j)) ; do
	if [ $j -eq 1 ] ; then
	    echo "Run "$teName $tpack $runOpt $noLoop
	else
	    echo "Retry($j) "$teName $tpack $runOpt $noLoop
	fi

	bash $toName-command-line$wantOpt-$tpack$wantlog$wantNoLoop.sh
	RETVAL=$?
	echo $RETVAL
	
	if [ $RETVAL -eq 0 ] ; then
	    echo "PASSED: $teName $tpack $runOpt $noLoop" > $toName-status.log
	    echo $toName,$teName,$tpack,$runOpt,$noLoop >> $basedir/$passedTestFile
	    (cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "PASSED: dmcs testcase on GLUCK: $teName $tpack $runOpt $noLoop" $EMAIL
	    break
	elif [ $RETVAL -eq 124 ] ; then
	    echo "TIMEOUT: $teName $tpack $runOpt $noLoop" > $toName-status.log
	    echo $toName,$teName,$tpack,$runOpt,$noLoop >> $basedir/$timeoutTestFile
	    (cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "TIMEOUT: dmcs testcase on GLUCK: $teName $tpack $runOpt $noLoop" $EMAIL
	    break
	else
	    if [ $j -eq $MAXTRY ] ; then
		echo "FAILED: $teName $tpack $runOpt $noLoop" > $toName-status.log
		echo $toName,$teName,$tpack,$runOpt,$noLoop >> $basedir/$failedTestFile
		(cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "FAILED: dmcs testcase on GLUCK: $teName $tpack $runOpt $noLoop" $EMAIL
	    else
		(cat $toName-time.log ; echo ; cat $toName.log ; echo ; cat $toName-err.log) | mail -s "FAILED($j): dmcs testcase on GLUCK: $teName $tpack $runOpt $noLoop" $EMAIL
	    fi
	fi
    done

    moveLogFiles $toName $teName $tpack $runOpt $noLoop
} # end of runOneInstance

#################################################################################
runAll()
{
    # $1: passedTestFile
    # $2: failedTestFile
    # $3: timeoutTestFile 
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
		runOneInstance $1 $2 $3 $toponame $testname ${testpack[$i]} noopt loop
		runOneInstance $1 $2 $3 $toponame $testname ${testpack[$i]} opt loop
		if [ $i -gt 1 ] ; then
		    runOneInstance $1 $2 $3 $toponame $testname ${testpack[$i]} noopt noloop
		    runOneInstance $1 $2 $3 $toponame $testname ${testpack[$i]} opt noloop
		fi
	    done
	    
	    cd ../../.. # get out of data/$toponame/$testname
	done
    done
} # end of runAll

#################################################################################
reRun()
{
    rerunCases=$1
    # $2: passedTestFile
    # $3: failedTestFile
    # $4: timeoutTestFile
    for line in `cat $rerunCases` ; do
	echo $line
	list=(`echo $line | tr ',' ' ' `)
	noArguments=${#list[@]}

	# list[0] = toponame
	# list[1] = testname
	# list[2] = testpack
	# list[3] = (opt-mode)
	# list[4] = (loop-mode)
	cd data/${list[0]}/${list[1]}
	if [ $noArguments -eq 5 ] ; then
	    runOneInstance $2 $3 $4 ${list[0]} ${list[1]} ${list[2]} ${list[3]} ${list[4]}
	elif [ $noArguments -eq 4 ] ; then
	    runOneInstance $2 $3 $4 ${list[0]} ${list[1]} ${list[2]} ${list[3]}
	else
	    runOneInstance $2 $3 $4 ${list[0]} ${list[1]} ${list[2]}
	fi
	cd ../../.. # get out of data/$toponame/$testname
    done
} # end of reRun()

#################################################################################
#
# extract command line arguments:
#
# [-r FILE] [-n DIRECTORY]
#
# -r:          rerun all FAILED testcases
# [FILE]:      filename to store FAILED test cases
# [DIRECTORY]: name of directory containing data

currentDateTime=$(date +"%m-%d-%Y-%H-%M-%S")
echo $currentDateTime

passedTestFile=passedTests-$currentDateTime.log
failedTestFile=failedTests-$currentDateTime.log
timeoutTestFile=timeoutTests-$currentDateTime.log

rerun=no
rerunCases=nocase
subdir=default

while [ $# -gt 0 ]
do
    case "$1" in
	-h) echo >&2 "Usage: $0 [-f FILE] [-r] [-n DIRECTORY]"
	    exit 0;;
	-r) rerun=yes; rerunCases="$2"; shift;; 
	-n) subdir="$2"; shift;;
	-*) echo >&2  "Usage: $0 [-f FILE] [-r] [-n DIRECTORY]"
	    exit 1;;
	*) break;;
    esac
    shift
done

cd $subdir

pwd

if [ x$rerun = xyes ] ; then
    reRun $rerunCases $passedTestFile $failedTestFile $timeoutTestFile
else
    runAll $passedTestFile $failedTestFile $timeoutTestFile
fi

cd ..