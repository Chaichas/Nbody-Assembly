all: nbody0 nbody1

nbody0: nbody0_cSOAopt.c
	gcc -g -Ofast -funroll-loops -finline-functions -ftree-vectorize $< -o $@ -lm -lSDL2 
	
	
nbody1: nbody0_c.c
	gcc -g -Ofast -funroll-loops -finline-functions -ftree-vectorize $< -o $@ -lm -lSDL2 

run: nbody0 nbody1
	sudo taskset -c 1 ./nbody0 > out0.dat
	sudo taskset -c 1 ./nbody1 > out1.dat
	gnuplot ./plot-nbody0SOA_AOS.gp > Comparison_SOAetAOS_opt.png

clean:
	rm -Rf *~ nbody0 nbody1 *.dat out.png
