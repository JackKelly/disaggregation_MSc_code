reset

# LaTeX output
SETTERMINAL
SETOUTPUT

set macros

plotHist = "\"HISTFILE\" with l lw 1 title \"HISTKEY\""
plotRAhist = "\"HISTGRADFILE\" with l lt 2 lw 1 title \"HISTGRADKEY\""
plotStates = "\"STATESFILE\" with xerrorbars title \"STATESKEY\""

set xlabel "XLABEL"
set ylabel "YLABEL"

# set y2range [y1/10:y2/10]
set y2label "gradient of histogram"
# set y2tics

plot [0:2500] @plotHist, @plotRAhist axis x1y2, @plotStates

clear
unset output # forces buffer to flush