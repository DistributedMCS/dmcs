#!/bin/bash

#TOPOLOGIES="tree diamond zigzag ring"
TOPOLOGIES="diamond ring"
INPUTEXT="out1 out2"

echo "\begin{landscape}" > tables-all.tex

for topo in $TOPOLOGIES ; do
    for inputext in $INPUTEXT ; do
	python tables.py --topo=$topo --inp=$inputext
	cat table-$topo-$inputext.tex >> tables-all.tex
	echo "" >> tables-all.tex
	echo "" >> tables-all.tex
	echo "" >> tables-all.tex
    done
done

echo "\end{landscape}" >> tables-all.tex