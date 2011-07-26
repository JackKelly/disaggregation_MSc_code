reset

# LaTeX output
SETTERMINAL
SETOUTPUT

set macros

unset y2label
unset y2tics
set x2zeroaxis
set y2zeroaxis

plotHist = "\"HISTFILE\" with l lw 1 title \"HISTKEY\""
plotRAhist = "\"HISTGRADFILE\" with l lt 2 lw 1 title \"HISTGRADKEY\""
plotStates = "\"STATESFILE\" with xerrorbars title \"STATESKEY\""

set xlabel "XLABEL"
set ylabel "YLABEL"

set border 1+2+4+8

set y2range [-0.005:0.005]
set y2label "gradient of histogram"
# set y2tics

plot [0:2700] [-0.1:0.1] @plotHist, @plotRAhist axis x1y2, @plotStates

clear
unset output # forces buffer to flush