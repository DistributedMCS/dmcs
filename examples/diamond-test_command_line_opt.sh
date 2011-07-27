#!/bin/bash
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S'
export TESTSPATH='.'
export DMCSPATH=../../../../../build-dbg/src
killall dmcsd
$DMCSPATH/dmcsd --context=1 --port=5001 --kb=$TESTSPATH/diamond-test-1.lp --br=$TESTSPATH/diamond-test-1.br --topology=$TESTSPATH/diamond-test.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=2 --port=5002 --kb=$TESTSPATH/diamond-test-2.lp --br=$TESTSPATH/diamond-test-2.br --topology=$TESTSPATH/diamond-test.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=3 --port=5003 --kb=$TESTSPATH/diamond-test-3.lp --br=$TESTSPATH/diamond-test-3.br --topology=$TESTSPATH/diamond-test.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=4 --port=5004 --kb=$TESTSPATH/diamond-test-4.lp --br=$TESTSPATH/diamond-test-4.br --topology=$TESTSPATH/diamond-test.opt >/dev/null 2>&1 &
sleep 20
/usr/bin/time --portability -o diamond-test-dmcs-opt-time.log $DMCSPATH/dmcsc --hostname=localhost --port=5001 --streaming=0 --system-size=4 > diamond-test-dmcs-opt.log 2> diamond-test-dmcs-opt-err.log
killall dmcsd
