#!/bin/bash
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S'
export TESTSPATH='tests'
export DMCSPATH='.'
$DMCSPATH/dmcsd --context=1 --port=5001 --kb=$TESTSPATH/diamond-4-4-2-2-a-1.lp --br=$TESTSPATH/diamond-4-4-2-2-a-1.br --topology=$TESTSPATH/diamond-4-4-2-2-a.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=2 --port=5002 --kb=$TESTSPATH/diamond-4-4-2-2-a-2.lp --br=$TESTSPATH/diamond-4-4-2-2-a-2.br --topology=$TESTSPATH/diamond-4-4-2-2-a.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=3 --port=5003 --kb=$TESTSPATH/diamond-4-4-2-2-a-3.lp --br=$TESTSPATH/diamond-4-4-2-2-a-3.br --topology=$TESTSPATH/diamond-4-4-2-2-a.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=4 --port=5004 --kb=$TESTSPATH/diamond-4-4-2-2-a-4.lp --br=$TESTSPATH/diamond-4-4-2-2-a-4.br --topology=$TESTSPATH/diamond-4-4-2-2-a.opt >/dev/null 2>&1 &
/usr/bin/time --portability -o diamond-4-4-2-2-a-dmcsopt-time.log $DMCSPATH/dmcsc --hostname=localhost --port=5001 --system-size=4 > diamond-4-4-2-2-a-dmcsopt.log 2> diamond-4-4-2-2-a-dmcsopt-err.log
killall dmcsd
