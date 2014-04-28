#!/bin/bash

TOPOLOGIES="tree diamond zigzag ring"
INPUTEXT="out1 out2 out3"

echo "\begin{landscape}" > tables-all.tex

for topo in $TOPOLOGIES ; do
    for inputext in $INPUTEXT ; do
	if [ ! -e config/$topo.$inputext ] ; then
	    echo Configuration $topo.$inputext does not exists. Going to ignore...	    
	else
	    python tables.py --topo=$topo --inp=$inputext
	    cat table-$topo-$inputext.tex >> tables-all.tex
	    echo "" >> tables-all.tex
	    echo "" >> tables-all.tex
	    echo "" >> tables-all.tex
	fi
    done
done

echo "\end{landscape}" >> tables-all.tex