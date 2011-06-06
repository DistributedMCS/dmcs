#!/bin/bash
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S'
export TESTSPATH=./
export DMCSPATH=`pwd`/../build/src/
$DMCSPATH/dmcsd --context=1 --port=5001 --kb=$TESTSPATH/dia-4-4-2-2-1-1.lp --br=$TESTSPATH/dia-4-4-2-2-1-1.br --topology=$TESTSPATH/dia-4-4-2-2-1.top >ctx1.log 2>&1 &
$DMCSPATH/dmcsd --context=2 --port=5002 --kb=$TESTSPATH/dia-4-4-2-2-1-2.lp --br=$TESTSPATH/dia-4-4-2-2-1-2.br --topology=$TESTSPATH/dia-4-4-2-2-1.top >ctx2.log 2>&1 &
$DMCSPATH/dmcsd --context=3 --port=5003 --kb=$TESTSPATH/dia-4-4-2-2-1-3.lp --br=$TESTSPATH/dia-4-4-2-2-1-3.br --topology=$TESTSPATH/dia-4-4-2-2-1.top >ctx3.log 2>&1 &
$DMCSPATH/dmcsd --context=4 --port=5004 --kb=$TESTSPATH/dia-4-4-2-2-1-4.lp --br=$TESTSPATH/dia-4-4-2-2-1-4.br --topology=$TESTSPATH/dia-4-4-2-2-1.top >ctx4.log 2>&1 &
sleep 1
tail -f ctx*.log &
sleep 5
time $DMCSPATH/dmcsc --hostname=localhost --port=5001 --system-size=4 --query-variables="18446744073709551615 17 13 23 "
killall dmcsd
