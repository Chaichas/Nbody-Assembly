/*
  N-BODY collision simulation
  
  Bad code --> optimize
  
*/

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <paths.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "rdtsc.h"

//#define N _NBODY_

//
typedef struct {

  double *x, *y;

} vector;


//
int w, h;

//
int nbodies, timeSteps;

//
double *masses, GravConstant;

//
vector positions, velocities, accelerations;

//
unsigned long long rdtsc(void) {
  unsigned long long a, d;

  __asm__ volatile("rdtsc": "=a"(a), "=d"(d));

  return (d << 32) | a;
}

//
int randxy(int x, int y) {
  return (rand() % (y - x + 1)) + x;
}

//
double randreal() {
  int s = (randxy(0, 1)) ? 1 : -1;
  int a = randxy(1, RAND_MAX), b = randxy(1, RAND_MAX);

  return s * ((double) a / (double) b);
}


//
//double mod(double x,double y)
//{
//  return sqrt(x * x + y * y);}

//
void init_system() {
  w = h = 800;
  nbodies = 500;
  GravConstant = 1;
  timeSteps = 1000;

  //
  masses = malloc(nbodies * sizeof(double));
  positions.x= malloc(nbodies * sizeof(double));
  positions.y= malloc(nbodies * sizeof(double));
  
  velocities.x= malloc(nbodies * sizeof(double));
  velocities.y= malloc(nbodies * sizeof(double));
  
  accelerations.x= malloc(nbodies * sizeof(double));
  accelerations.y= malloc(nbodies * sizeof(double));

  //
  for (int i = 0; i < nbodies; i++) {
    masses[i] = 5;

    positions.x[i] = randxy(10, w);
    positions.y[i] = randxy(10, h);

    velocities.x[i] = randreal();
    velocities.y[i] = randreal();
  }
}

//
void resolve_collisions() {
  //
  for (int i = 0; i < nbodies - 1; i++){
  
    for (int j = i + 1; j < nbodies; j++){
    
      if (positions.x[i] == positions.x[j] &&
        positions.y[i] == positions.y[j]) {
        
        double temp = velocities.x[i];
        velocities.x[i] = velocities.x[j];
        velocities.x[j] = temp;

        double temp1 = velocities.y[i];
        velocities.y[i] = velocities.y[j];
        velocities.y[j] = temp1;
      }
    }
  }
}

void compute_accelerations() {
  for (int i = 0; i < nbodies; i++) {
  
    accelerations.x[i] = 0;
    accelerations.y[i] = 0;
    
  for(int j = 0; j < nbodies; j++){
	if(i != j){
	
	double sub1= positions.x[j] - positions.x[i];
	double sub2= positions.y[j] - positions.y[i];
	double mod= sqrt(sub1*sub1+sub2*sub2);
	
	accelerations.x[i] = accelerations.x[i] + (GravConstant * masses[j] / (pow(mod, 3) + 1e7)) * sub1;  
	
	accelerations.y[i] = accelerations.y[i] + (GravConstant * masses[j] / (pow(mod, 3) + 1e7)) * sub2;
}      
}
}}

//
void compute_velocities() {
for (int i = 0; i < nbodies; i++){

  velocities.x[i] = velocities.x[i] + accelerations.x[i];
  velocities.y[i] = velocities.y[i] + accelerations.y[i];}
}

//
void compute_positions() {
for (int i = 0; i < nbodies; i++){

  positions.x[i] = positions.x[i] + velocities.x[i] + (0.5 * accelerations.x[i]);
  positions.y[i] = positions.y[i] + velocities.y[i] + (0.5 * accelerations.y[i]);}
}

//
void simulate() {
  compute_accelerations();
  compute_positions();
  compute_velocities();
  resolve_collisions();
}

//
int main(int argc, char ** argv) {
  //
  int i;
  unsigned char quit = 0;
  SDL_Event event;
  SDL_Window * window;
  SDL_Renderer * renderer;

  srand(time(NULL));

  //
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(800, 800, SDL_WINDOW_OPENGL, & window, & renderer);

  //
  init_system();

  //Main loop

  //FILE * imprimc;
  //imprimc = fopen("out0_SOAopt.dat", "wt");
  for (int i = 0; !quit && i < timeSteps; i++) {
    //
    double before = (double) rdtsc();

    simulate();

    //
    double after = (double) rdtsc();

    //
    printf("%d %lf\n", i, (after - before));
    //fprintf(imprimc, "%d %f \n", i, (after - before));

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < nbodies; i++) {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderDrawPoint(renderer, positions.x[i], positions.y[i]);
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(10);

    while (SDL_PollEvent( & event))
      if (event.type == SDL_QUIT)
        quit = 1;
      else
    if (event.type == SDL_KEYDOWN)
      if (event.key.keysym.sym == SDLK_q)
        quit = 1;
  }
  //fclose(imprimc);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
