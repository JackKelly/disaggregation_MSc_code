SETTERMINAL
SETOUTPUT
set title "TITLE"
set xlabel "XLABEL"
set ylabel "YLABEL"
set yrange [0:3500]
set xdata time
set timefmt "%s"
set format x "%d/%m\n%H:%M"
plot PLOTARGS "DISAGGFILE" using ($1+3600):2 with filledcurves lw 1 t "DISAGGKEY", \
"data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv" using ($1+3600):2 with l lw 1 t "Raw aggregate signal"
