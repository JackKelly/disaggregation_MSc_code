SETTERMINAL
SETOUTPUT
set title "TITLE"
set xlabel "XLABEL"
set ylabel "YLABEL"
set yrange [0:3500]
set xdata time
set timefmt "%s"
set format x "%d/%m\n%H:%M"

set macros
dstOffset = 3600 # to correct for BST

plot PLOTARGS "DISAGGFILE" using ($1+dstOffset):2 with filledcurves lw 1 t "DISAGGKEY", \
"AGGDATAFILE" using ($1+dstOffset):2 with l lw 1 t "AGGDATAKEY"
