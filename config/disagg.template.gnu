SETTERMINAL
SETOUTPUT
set title "TITLE"
set xlabel "XLABEL"
set ylabel "YLABEL"
set yrange [0:3500]
set xdata time
set timefmt "%s"
# set xrange ["1310294235+3600":"1310300931+3600"]
set format x "%d/%m\n%H:%M"
plot PLOTARGS "DISAGGFILE" using ($1+3600):2 with filledcurve t "DISAGGKEY", \
"data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv" using ($1+3600):2 with l
