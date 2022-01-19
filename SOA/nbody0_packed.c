/*
  N-BODY collision simulation
  
  Bad code --> optimize
  
*/

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "rdtsc.h"
 //
#define N 500

//
typedef struct {

  double x[N], y[N];

}
vector;

//
int w, h;
int i, j;

//
int nbodies, timeSteps;

//
double *masses, GravConstant;

//
vector positions, velocities, accelerations;

//
unsigned long long rdtsc(void)
{
  unsigned long long a, d;
  
  __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
  
  return (d << 32) | a;
}

//
int randxy(int x, int y)
{
  return (rand() % (y - x + 1)) + x; 
}

//
double randreal()
{
  int s = (randxy(0, 1)) ? 1 : -1;
  int a = randxy(1, RAND_MAX), b = randxy(1, RAND_MAX);

  return s * ((double)a / (double)b); 
}

//

vector add_vectors(vector a, vector b)
{

vector c;

//Size in bytes
  unsigned long long sizec = sizeof(double) * N;
  
//
__asm__ volatile (
		    "xorpd %%xmm2, %%xmm2;\n" //Logical Exclusive OR
		    "1:;\n" //loop
		    
		    "movapd (%[_a], %%xmm2), %%xmm0;\n" //xmm0 =  a.x[i] | a.y[i] 
		    "movapd (%[_b], %%xmm2), %%xmm1;\n" //xmm1 =  b.x[i] | b.y[i] 
		    
		    "addpd %%xmm1, %%xmm0;\n"          // a.x[i] + b.x[i], | a.y[i] + b.y[i]
		    "movapd %%xmm0, (%[_c], %%xmm2);\n"       //c.x[i] = a.x[i] + b.x[i], | c.y[i] =a.y[i] + b.y[i]
		    
		    "add $8, %%xmm2;\n"             //xmm2+= sizeof(double)
		    "cmp %[_sizec], %%xmm2;\n"
		    "jl 1b;\n"
		    
		    : //outputs
		      
		    : //inputs
		      [_a] "r" (&a),
		      [_b] "r" (&b),
		      [_c] "r" (&c),
		      [_sizec] "r" (sizec)
		      
		    : //clobbers
		      "cc", "memory", "xmm0", "xmm1", "xmm2"
		    );
  //
  return c;
}

//
vector scale_vector(double b, vector a)
{

  vector c;
  double B[2] = { b, b};
  
  //Size in bytes
  unsigned long long sizec = sizeof(double) * N;
  
//
__asm__ volatile (

		    "xorpd %%xmm1, %%xmm1;\n" //Logical Exclusive OR
		    "1:;\n" //loop
		    
		    "movapd (%[_a], %%xmm1), %%xmm0;\n"          //xmm0 =      a.x[i] | a.y[i] 
		    "mulpd (%[_b]), %%xmm0\n"          //xmm0 =     b * a.x[i] |     b * a.y[i]
		    "movapd %%xmm0, (%[_c], %%xmm1);\n" //c=     b * a.x[i] |     b * a.y[i] 
		    
		    "add $8, %%xmm1;\n"             //xmm1+= sizeof(double)
		    "cmp %[_sizec], %%xmm1;\n"
		    "jl 1b;\n"
  
		    : //outputs
		      
		    : //inputs
		      [_a] "r" (&a),
		      [_b] "r" (B),
		      [_c] "r" (&c),
		      [_sizec] "r" (sizec)
		      
		    : //clobbers
		      "cc", "memory", "xmm0", "xmm1"
		    );
  //
  return c;
}

//
vector sub_vectors(vector a, vector b)
{
  
  vector c;
  
  //Size in bytes
  unsigned long long sizec = sizeof(double) * N;
  
//
__asm__ volatile (
		    "xorpd %%xmm2, %%xmm2;\n" //Logical Exclusive OR
		    "1:;\n" //loop
		    
		    "movapd (%[_a], %%xmm2), %%xmm0;\n" //xmm0 =  a.x[i] | a.y[i] 
		    "movapd (%[_b], %%xmm2), %%xmm1;\n" //xmm1 =  b.x[i] | b.y[i] 
		    
		    "subpd %%xmm1, %%xmm0;\n"          // a.x[i] + b.x[i], | a.y[i] + b.y[i]
		    "movapd %%xmm0, (%[_c], %%xmm2);\n"       //c.x[i] = a.x[i] + b.x[i] | c.y[i] =a.y[i] + b.y[i]
		    
		    "add $8, %%xmm2;\n"             //xmm2+= sizeof(double)
		    "cmp %[_sizec], %%xmm2;\n"
		    "jl 1b;\n"
		    
		    : //outputs
		      
		    : //inputs
		      [_a] "r" (&a),
		      [_b] "r" (&b),
		      [_c] "r" (&c),
		      [_sizec] "r" (sizec)
		      
		    : //clobbers
		      "cc", "memory", "xmm0", "xmm1", "xmm2"
		    );
  
  return c;
}

//
double mod(vector a)
{
  double m=0.0;

  //Size in bytes
  unsigned long long sizec = sizeof(double) * N;
  
  __asm__ volatile (
  
		    "xorpd %%xmm2, %%xmm2;\n" //Logical Exclusive OR
		    "1:;\n" //loop
		    
		    "movapd (%[_a], %%xmm2), %%xmm0;\n"          //xmm0 = a.x[i] | a.y[i]
		    "mulpd %%xmm0, %%xmm0\n"          //xmm0 = a.x[i] * a.x[i] |     a.x[i] * a.y[i] 
		    "movapd %%xmm0, %%xmm1;\n" //xmm1= xmm0
		    "haddpd %%xmm1, %%xmm0;\n" //xmm0= a.x[i] * a.x[i] + a.y[i] * a.y[i]
		    "sqrtsd %%xmm0, %%xmm0;\n" //sqrt(a.x[i] * a.x[i] + a.y[i] * a.y[i])
		    "movsd %%xmm0, (%[_m], %%xmm2);\n" //m= sqrt(a.x[i] * a.x [i]+ a.y[i] * a.y[i])
    
		    : //outputs
		      
		    : //inputs
		      [_a] "r" (&a),
		      [_m] "r" (&m),
		      [_sizec] "r" (sizec)
		      
		    : //clobbers
		      "cc", "memory", "xmm0", "xmm1", "xmm2"
		    );
  
  return m;    
    
}

//
void init_system() {
  w = h = 800;
  nbodies = 500;
  GravConstant = 1;
  timeSteps = 1000;

  //
  masses = malloc(nbodies * sizeof(double));

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
  for (int i = 0; i < nbodies - 1; i++)
    for (int j = i + 1; j < nbodies; j++)
      if (positions.x[i] == positions.x[j] &&
        positions.y[i] == positions.y[j]) {
        double temp = velocities.x[i];
        velocities.x[i] = velocities.x[j];
        velocities.x[j] = temp;

        double temp2 = velocities.y[i];
        velocities.y[i] = velocities.y[j];
        velocities.y[j] = temp2;
      }
}

void compute_accelerations() {
  for (i = 0; i < nbodies; i++) {
  
    accelerations.x[i] = 0;
    accelerations.y[i] = 0;
  if (i != j)
  accelerations = add_vectors(accelerations,
    scale_vector(GravConstant * masses[j] / (pow(mod(sub_vectors(positions, positions)), 3) + 1e7),
      sub_vectors(positions, positions)));

}
}

//
void compute_velocities() {
  velocities = add_vectors(velocities, accelerations);
}

//
void compute_positions() {
  positions = add_vectors(positions, add_vectors(velocities, scale_vector(0.5, accelerations)));
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

  FILE * imprimc;
  imprimc = fopen("out0_SOA.dat", "wt");
  for (int i = 0; !quit && i < timeSteps; i++) {
    //
    double before = (double) rdtsc();

    simulate();

    //
    double after = (double) rdtsc();

    //
    printf("%d %lf\n", i, (after - before));
    fprintf(imprimc, "%d %f \n", i, (after - before));

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < nbodies; i++) {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      //  printf("x \n %lf y \n %lf",positions[2].x, positions[2].y);
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
  fclose(imprimc);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
