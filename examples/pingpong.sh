#!/bin/bash
dmcs=${DMCS:=./src}
log4cxx=${LOG4CXX:=.}
examplesdir=${EXAMPLESDIR:=./examples}
$dmcs/dmcsd --daemon --logging=$log4cxx/log4cxx.cfg --context=1 --port=5001 --kb=$examplesdir/pingpong1.lp --br=$examplesdir/pingpong1.br --topology=$examplesdir/pingpong.opt
$dmcs/dmcsd --daemon --logging=$log4cxx/log4cxx.cfg --context=2 --port=5002 --kb=$examplesdir/pingpong2.lp --br=$examplesdir/pingpong2.br --topology=$examplesdir/pingpong.opt
time $dmcs/dmcsc --hostname=localhost --port=5001 --system-size=2
