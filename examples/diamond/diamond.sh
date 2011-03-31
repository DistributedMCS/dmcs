#!/bin/bash
dmcs=${DMCS:=./src}
log4cxx=${LOG4CXX:=.}
examplesdir=${EXAMPLESDIR:=./examples}
$dmcs/dmcsd --daemon --logging=$log4cxx/log4cxx.cfg --context=1 --port=5001 --kb=$examplesdir/diamond1.lp --br=$examplesdir/diamond1.br --topology=$examplesdir/diamond.opt
$dmcs/dmcsd --daemon --logging=$log4cxx/log4cxx.cfg --context=2 --port=5002 --kb=$examplesdir/diamond2.lp --br=$examplesdir/diamond2.br --topology=$examplesdir/diamond.opt
$dmcs/dmcsd --daemon --logging=$log4cxx/log4cxx.cfg --context=3 --port=5003 --kb=$examplesdir/diamond3.lp --br=$examplesdir/diamond3.br --topology=$examplesdir/diamond.opt
$dmcs/dmcsd --daemon --logging=$log4cxx/log4cxx.cfg --context=4 --port=5004 --kb=$examplesdir/diamond4.lp --br=$examplesdir/diamond4.br --topology=$examplesdir/diamond.opt
time $dmcs/dmcsc --hostname=localhost --port=5001 --system-size=4
