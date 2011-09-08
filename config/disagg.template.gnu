SETTERMINAL
SETOUTPUT
# set title "TITLE"
set xlabel "XLABEL"
set ylabel "YLABEL"
set yrange [0:3.7]
set xdata time
set timefmt "%s"
set format x "%d/%m\n%H:%M"
set tics out
set xtics nomirror
set mxtics 3

unset key
set macros
dstOffset = 3600 # to correct for BST

set border 1+2+0+8 lc rgb "grey"

plot PLOTARGS "DISAGGFILE" using ($1+dstOffset):($2/1000) with filledcurves lw 1 lc rgb "blue" t "DISAGGKEY", \
"AGGDATAFILE" using ($1+dstOffset):($2/1000) with l lw 1 lc rgb "black" t "AGGDATAKEY"
