set grid
set ylabel "Latency in cycles"

set xlabel "Simulation iteration"
plot "out0_SOA.dat" title "C version", "out0_sd.dat" title "SSE scalar", "out0_pd.dat" title "SSE packed"
pause -1
