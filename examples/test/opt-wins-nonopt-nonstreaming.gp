#TODO: decide test cases to be put in this comparison

set term postscript eps enhanced font "Helvetica,16" size 18cm,6cm
set termoption font "Helvetica,16"
set out "opt-wins-nonopt-nonstreaming.eps"

set key at 0.5,800

set logscale y

set style data histogram 
set style histogram gap 0

unset xtics
set xtics nomirror norotate font ",0.01"

set style fill solid border 0
set style line 1 lc rgb "blue" linewidth -1 linetype 1
set style line 2 lc rgb "green" linewidth -1 linetype 1
set style line 3 lc rgb "blue" linewidth -1 linetype 1
set style line 4 lc rgb "green" linewidth -1 linetype 1
set style line 5 lc rgb "blue" linewidth -1 linetype 1
set style line 6 lc rgb "green" linewidth -1 linetype 1
set style line 7 lc rgb "blue" linewidth -1 linetype 1
set style line 8 lc rgb "green" linewidth -1 linetype 1
set style increment user

plot newhistogram "D(4,10,5,5)" lt 1, '../data/diamond-4-10-5-5.dat' using 1 title "DMCS", '' using 3 title "DMCSOPT",\
     newhistogram "D(7,10,5,5)" lt 1,  '../data/diamond-7-10-5-5.dat' using 1 notitle, '' using 3 notitle,\
     newhistogram "D(10,10,5,5)" lt 1, '../data/diamond-10-10-5-5.dat' using 1 notitle, '' using 3 notitle,\
     newhistogram "D(13,10,5,5)" lt 1, '../data/diamond-13-10-5-5.dat' using 1 notitle, '' using 3 notitle

!epstopdf opt-wins-nonopt-nonstreaming.eps && rm opt-wins-nonopt-nonstreaming.eps
exit