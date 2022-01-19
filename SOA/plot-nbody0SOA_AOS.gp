set grid
set ylabel "Latency in cycles"

set xlabel "Simulation iteration"
plot "out0.dat" title "C_{AOS}", "out1.dat" title "C_{SOA}"
pause -1
