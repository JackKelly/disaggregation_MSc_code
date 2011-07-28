SETTERMINAL
SETOUTPUT
set title "TITLE"
set xlabel "XLABEL"
set ylabel "YLABEL"

plot [0:5000] "SIGFILE" with l lw 1 lc "black" t "SIGKEY", "POWERSTATESEQUENCEFILE" with boxxyerrorbars lw 1 t "POWERSTATESEQUENCEKEY"

clear
unset output # forces buffer to flushb