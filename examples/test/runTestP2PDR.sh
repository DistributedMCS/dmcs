#!/bin/bash

export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S'
export TIMEOUT=600

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
    basedir=../..
    mvTopoName=$1
    mvInstanceName=$2

    createSubSir $basedir/output-p2p-dr
    createSubSir $basedir/output-p2p-dr/$mvTopoName
    createSubSir $basedir/output-p2p-dr/$mvTopoName/$mvInstanceName

    rm $basedir/output-p2p-dr/$mvTopoName/$mvInstanceName/*

    mv *.log $basedir/output-p2p-dr/$mvTopoName/$mvInstanceName
} # end moveLogFiles

#################################################################################
runOneInstance()
{
    killall java

    echo "Sleeping for 5 secs after killing all java processes..."
    sleep 5

    # start Services

    cd p2p-dr/services
    java Services &
    
    # start peers
    cd ../peer

    rm peer*_rules.txt
    rm peer*_trust.txt

    topo=$1
    instance=$2

    # copy all rules and trust files to the peer directory
    # ToDo: move p2p_dr to test and use relative path
    cp /Users/minhdt/Documents/softwares/dmcs/examples/test/data-p2p-dr/$topo/$instance/*.* .

    list=(`echo $instance | tr '-' ' '`)
    #list[0]=topo
    #list[1]=system size
    #...

    system_size=${list[1]}

    for ((i = 0; i < $system_size; ++i)) ; do
	port=$((5000+i))
	
	java Node peer$i $port localhost 1 &
    done

    # query at client
    # ToDo: test on gluck the time command
    #/usr/bin/time --verbose -o $topo-time.log /usr/bin/timeout -k 20 $TIMEOUT java Client peer0 localhost c0a1

    time java Client peer0 localhost c0a1

    moveLogFiles $topo $instance

    cd ../.. # get out

} # end runOneInstance


#################################################################################

for topofile in `ls config/*.cfg` ; do
    length=${#topofile}
    let "length -=11"
    toponame=${topofile:7:$length}
    
    for line in `cat $topofile` ; do
	list=(`echo $line | tr ',' ' ' `)
	instance=$toponame-${list[1]}-${list[2]}-${list[3]}-${list[4]}-${list[5]}
	
	runOneInstance $toponame $instance
    done
done
