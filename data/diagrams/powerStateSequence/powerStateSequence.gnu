# reset

# LaTeX output
# set terminal epslatex solid colour size 15cm,12cm font "" 8
# set output "histAndStatesAndRAgradient.tex"

# plotHist = "\"hist.dat\" with l lw 1 title \"histogram\""
# plotRAhist = "\"RA_hist.dat\" with l lt 2 lw 1 title \"31-step rolling average of histogram gradient\""
# plotXerrbars = "\"x_err_bars.dat\" with xerrorbars title \"automatically determined state boundaries (min, mean, max)\""

plot "powerStateSequence.dat" with boxxyerrorbars

# clear
# unset output # forces buffer to flushb