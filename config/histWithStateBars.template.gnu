reset

# LaTeX output
SETTERMINAL
SETOUTPUT

unset y2label
unset y2tics
unset x2zeroaxis
set macros
bm = 0.15
tm = 0.17
lm = 0.12
rm = 0.89
gap = 0.02
size = 0.75
y1 = -30.0; y2 = 90; y3 = 2747; y4 = 2765
x1 =  0.0; x2 = 299; x3 = 2155; x4 = 2350 

plotHist = "\"HISTFILE\" with l lw 1 title \"HISTKEY\""
plotRAhist = "\"HISTGRADFILE\" with l lt 2 lw 1 title \"HISTGRADKEY\""
plotXerrbars = "\"STATESFILE\" with xerrorbars title \"STATESKEY\""

set multiplot

###################
# Bottom left panel
unset title
unset key
set border 1+2
set xtics nomirror
set ytics nomirror
set lmargin at screen lm
set rmargin at screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) )
set bmargin at screen bm
set tmargin at screen bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) )

set yrange [y1:y2]
set xrange [x1:x2]
set y2range [y1/10:y2/10]

plot @plotHist, @plotRAhist axis x1y2, @plotXerrbars

################
# Top left panel
unset key
unset xtics
set ytics 10
unset xlabel
unset ylabel
set border 2+4
set bmargin at screen bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) ) + gap
set tmargin at screen bm + size + gap
set yrange [y3:y4]
set xrange [x1:x2]

plot @plotHist, @plotRAhist, @plotXerrbars

#################
# Top right panel
set key # height 20 
unset xtics
unset ytics
unset xlabel
unset ylabel
set border 4+8
set bmargin at screen bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) ) + gap
set tmargin at screen bm + size + gap
set rmargin at screen lm + size + gap
set lmargin at screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) + gap
set yrange [y3:y4]
set xrange [x3:x4]

plot @plotHist, @plotRAhist, @plotXerrbars

################
# Bottom right panel
unset key
set xtics 50
unset ytics

unset xlabel
unset ylabel
set xtics nomirror
set border 1+8
set bmargin at screen bm
set tmargin at screen bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) )
set rmargin at screen lm + size + gap
set lmargin at screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) + gap
set yrange [y1:y2]
set xrange [x3:x4]

set y2range [y1/10:y2/10]
set y2label "Gradient of histogram"
set y2tics



#############################
# Axis Labels and graph Title

#set label 'TITLE' at \
#screen 0.50, \
#screen 0.96 \
#center

set label 'YLABEL' at \
screen 0.06, \
bm + 0.5 * (size + gap) \
rotate by 90 \
center

set label 'XLABEL' at \
screen 0.50, \
0.08 \
center


########################
# break marks for y axis
set arrow from screen lm - (gap / 4.0), \
bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) ) - (gap / 4.0) \
to screen lm + (gap / 4.0), \
bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) ) + (gap / 4.0) nohead

set arrow from screen lm - (gap / 4.0), \
bm + size * (abs(y2-y1) / (abs(y2-y1)+abs(y4-y3) ) ) - (gap / 4.0) + gap \
to screen lm + (gap / 4.0), \
bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) ) + (gap / 4.0) + gap nohead

set arrow from screen rm - (gap / 4.0), \
bm + size * (abs(y2-y1) / (abs(y2-y1)+abs(y4-y3) ) ) - (gap / 4.0) \
to screen rm + (gap / 4.0), \
bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) ) + (gap / 4.0) nohead

set arrow from screen rm - (gap / 4.0), \
bm + size * (abs(y2-y1) / (abs(y2-y1)+abs(y4-y3) ) ) - (gap / 4.0) + gap \
to screen rm + (gap / 4.0), \
bm + size * (abs(y2-y1) / (abs(y2-y1) + abs(y4-y3) ) ) + (gap / 4.0) + gap nohead


########################
# break marks for x axis

# top border
# left
set arrow from screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) - (gap / 8.0), \
tm + size - (gap / 4.0) \
to screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) + (gap / 8.0), \
tm + size + (gap / 4.0) nohead

#right
set arrow from screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) - (gap / 8.0) + gap, \
tm + size - (gap / 4.0) \
to screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) + (gap / 8.0) + gap, \
tm + size + (gap / 4.0) nohead

# bottom border
# left
set arrow from screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) - (gap / 8.0), \
bm - (gap / 4.0) \
to screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) + (gap / 8.0), \
bm + (gap / 4.0) nohead

# right
set arrow from screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) - (gap / 8.0) + gap, \
bm - (gap / 4.0) \
to screen lm + size * (abs(x2-x1) / (abs(x2-x1) + abs(x4-x3) ) ) + (gap / 8.0) + gap, \
bm + (gap / 4.0) nohead

plot @plotHist, @plotRAhist axis x1y2, @plotXerrbars

unset multiplot

clear
unset output # forces buffer to flush