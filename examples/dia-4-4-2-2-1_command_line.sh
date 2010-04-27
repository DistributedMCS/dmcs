#!/bin/bash
export TIMEFORMAT=$'\nreal\t%3R\nuser\t%3U\nsys\t%3S'
export TESTSPATH='tests'
export DMCSPATH='.'
$DMCSPATH/dmcsd --context=1 --port=5001 --kb=$TESTSPATH/dia-4-4-2-2-1-1.lp --br=$TESTSPATH/dia-4-4-2-2-1-1.br --topology=$TESTSPATH/dia-4-4-2-2-1.top >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=2 --port=5002 --kb=$TESTSPATH/dia-4-4-2-2-1-2.lp --br=$TESTSPATH/dia-4-4-2-2-1-2.br --topology=$TESTSPATH/dia-4-4-2-2-1.top >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=3 --port=5003 --kb=$TESTSPATH/dia-4-4-2-2-1-3.lp --br=$TESTSPATH/dia-4-4-2-2-1-3.br --topology=$TESTSPATH/dia-4-4-2-2-1.top >/dev/null 2>&1 &
$DMCSPATH/dmcsd --context=4 --port=5004 --kb=$TESTSPATH/dia-4-4-2-2-1-4.lp --br=$TESTSPATH/dia-4-4-2-2-1-4.br --topology=$TESTSPATH/dia-4-4-2-2-1.top >/dev/null 2>&1 &
time $DMCSPATH/dmcsc --hostname=localhost --port=5001 --system-size=4 --query-variables="18446744073709551615 1 1 1 "
killall dmcsd
