SETTERMINAL
SETOUTPUT
set title "TITLE"
set xlabel "XLABEL"
set ylabel "YLABEL"

set tics out textcolor rgb "black"
set ytics 0.5
set mytics
set xtics nomirror
unset key
set border 1+2+0+8 lc rgb "grey"

plot "DATAFILE" using 0:($1/1000) with l lw 1 lc rgb "black" t "DATAKEY"
