#!/bin/bash

DMCSPATH="../"
TESTSPATH="./tests"

PREFIX=$1
N=$2
S=$3
B=$4
R=$5
NOTESTS=$(($6))
T=$7
TEMPLATE=$PREFIX-$N-$S-$B-$R

DOPRIMITIVE=$8

test -d $TESTSPATH || mkdir $TESTSPATH
cd $TESTSPATH


for (( i=1; i<=$NOTESTS; i++ ))
do
    c=$(echo {a..z} | awk "{print \$$i;}")
    echo Generating $TEMPLATE-$c...
    $DMCSPATH/dmcsGen --contexts=$N --atoms=$S --interface=$B --bridge_rules=$R --topology=$T --template=$TEMPLATE-$c
    sleep 1
done

cd ..

#
#for (( i=1; i<=$NOTESTS; i++ ))
#do
#    c=$(echo {a..z} | awk "{print \$$i;}")
#
#    THETEST=$TEMPLATE-$c
#
#    if $DOPRIMITIVE ; then
#	echo calling DMCS $THETEST...
#	date > $THETEST-dmcs-start.log
#	bash $TESTSPATH/$THETEST"_command_line.sh" 2>/dev/null
#	sleep 10
#    fi
#
#    echo calling DMCSOPT $THETEST...
#    date > $THETEST-dmcsopt-start.log
#    bash $TESTSPATH/$THETEST"_command_line_opt.sh" 2>/dev/null
#    sleep 10
#
#done
#
### do csv here...
##
#
#ZIPDIR=$TEMPLATE-$NOTESTS
#echo Generating $ZIPDIR.zip...
#mkdir $ZIPDIR
#cp -v $TESTSPATH/$TEMPLATE-*-*.* *.log *.csv $ZIPDIR
#zip -r $ZIPDIR.zip $ZIPDIR
