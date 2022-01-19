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
typedef struct {

  double x, y;
  
} vector;

//
int w, h;

//
int nbodies, timeSteps;

//
double *masses, GravConstant;

//
vector *positions, *velocities, *accelerations;

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

//
__asm__ volatile (

		    "movsd (%[_a]), %%xmm0;\n"          //xmm0 =      a.x 
		    "movsd 8(%[_a]), %%xmm2;\n"          //xmm2 =     a.y 
		    
		    "movsd (%[_b]), %%xmm1;\n"          //xmm1 =      b.x 
		    "movsd 8(%[_b]), %%xmm3;\n"          //xmm3 =      b.y
		    
		    "addsd %%xmm1, %%xmm0;\n"           //xmm0 = a.x + b.x
		    "addsd %%xmm3, %%xmm2;\n"           //xmm2 = a.y + b.y
		    
		    "movsd %%xmm0, (%[_c]);\n"         //cx = a.x + b.x
		    "movsd %%xmm2, 8(%[_c]);\n"         //cy = a.y + b.y
  
		    : //outputs
		      
		    : //inputs
		      [_a] "r" (&a),
		      [_b] "r" (&b),
		      [_c] "r" (&c)
		      
		    : //clobbers
		      "cc", "memory", "xmm0", "xmm1", "xmm2", "xmm3"
		    );
  
  return c;
}

//
vector scale_vector(double b, vector a)
{
  //vector c = { b * a.x, b * a.y };
  vector c;
//
__asm__ volatile (

		    "movsd (%[_a]), %%xmm0;\n"          //xmm0 =      a.x 
		    "movsd 8(%[_a]), %%xmm2;\n"         //xmm2 =     a.y 
		    
		    "mulsd (%[_b]), %%xmm0\n"          //xmm0 =     b * a.x 
		    "mulsd (%[_b]), %%xmm2\n"          //xmm2 =     b * a.x 
		    
		    "movsd %%xmm0, (%[_c]);\n" //c=     b * a.x 
		    "movsd %%xmm2, 8(%[_c]);\n" //c=     b * a.y 
  
		    : //outputs
		      
		    : //inputs
		      [_a] "r" (&a),
		      [_b] "r" (&b),
		      [_c] "r" (&c)
		      
		    : //clobbers
		      "cc", "memory", "xmm0", "xmm2"
		    );
  
  return c;
}

//
vector sub_vectors(vector a, vector b)
{
  //vector c = { a.x - b.x, a.y - b.y };
  vector c;
//
__asm__ volatile (

		    "movsd (%[_a]), %%xmm0;\n"          //xmm0 =      a.x 
		    "movsd 8(%[_a]), %%xmm2;\n"          //xmm2 =      a.y 
		    
		    "movsd (%[_b]), %%xmm1;\n"          //xmm1 =      b.x
		    "movsd 8(%[_b]), %%xmm3;\n"         //xmm3 =      b.y  
		    
		    "subsd %%xmm1, %%xmm0;\n"           //xmm0 = a.x - b.x
		    "subsd %%xmm3, %%xmm2;\n"           //xmm2 = a.y - b.y
		    
		    "movsd %%xmm0, (%[_c]);\n"         //cx = a.x - b.x
		    "movsd %%xmm2, 8(%[_c]);\n"         //cy = a.y - b.y
  
		    : //outputs
		      
		    : //inputs
		      [_a] "r" (&a),
		      [_b] "r" (&b),
		      [_c] "r" (&c)
		      
		    : //clobbers
		      "cc", "memory", "xmm0", "xmm1", "xmm1", "xmm2", "xmm3"
		    );
  
  return c;
}

//
double mod(vector a)
{
  double m=0.0;

  //sqrt(a.x * a.x + a.y * a.y);
  __asm__ volatile (
  
		    "movsd (%[_a]), %%xmm0;\n"          //xmm0 =      a.x 
		    "movsd 8(%[_a]), %%xmm2;\n"          //xmm2 =      a.y, il faut se decaler de 8 bits
		    
		    "mulsd %%xmm0, %%xmm0\n"          //xmm0 =     a.x * a.x
		    "mulsd %%xmm2, %%xmm2\n"          //xmm2 =     a.y * a.y
		    
		    "movsd %%xmm0, %%xmm1;\n" //xmm1= xmm0
		    "movsd %%xmm2, %%xmm3;\n" //xmm3= xmm2
		    
		    "addsd %%xmm3, %%xmm1;\n" //xmm1= a.x * a.x + a.y * a.y
		    "sqrtsd %%xmm1, %%xmm1;\n" //sqrt(a.x * a.x + a.y * a.y)
		    
		    "movsd %%xmm1, (%[_m]);\n" //m= sqrt(a.x * a.x + a.y * a.y)
    
		    : //outputs
		      
		    : //inputs
		      [_a] "r" (&a),
		      [_m] "r" (&m)
		      
		    : //clobbers
		      "cc", "memory", "xmm0", "xmm1", "xmm2", "xmm3"
		    );
  
  return m;    
    
}

//
void init_system()
{
  w = h = 800;
  nbodies = 500;
  GravConstant = 1;
  timeSteps = 1000;
  
  //
  masses        = malloc(nbodies * sizeof(double));
  positions     = malloc(nbodies * sizeof(vector));
  velocities    = malloc(nbodies * sizeof(vector));
  accelerations = malloc(nbodies * sizeof(vector));

  //
  for (int i = 0; i < nbodies; i++)
    {
      masses[i] = 5;
      
      positions.x[i] = randxy(10, w);
      positions.y[i] = randxy(10, h);
      
      velocities.x[i] = randreal();
      velocities.y[i] = randreal();
    }
}

//
void resolve_collisions()
{
  //
  for (int i = 0; i < nbodies - 1; i++)
    for (int j = i + 1; j < nbodies; j++)
      if (positions.x[i] == positions.x[j] &&
	  positions.y[i] == positions.y[j])
	{
	  vector temp = velocities[i];
	  velocities[i] = velocities[j];
	  velocities[j] = temp;
	}
}

//
void compute_accelerations()
{ 
  for (int i = 0; i < nbodies; i++)
    {
      accelerations.x[i] = 0;
      accelerations.y[i] = 0;
      
      for(int j = 0; j < nbodies; j++)
	if(i != j)
	  accelerations[i] = add_vectors(accelerations[i],
					 scale_vector(GravConstant * masses[j] / (pow(mod(sub_vectors(positions[i], positions[j])), 3) + 1e7),
						      sub_vectors(positions[j], positions[i])));
    }
}

//
void compute_velocities()
{  
  for (int i = 0; i < nbodies; i++)
    velocities[i] = add_vectors(velocities[i], accelerations[i]);
}

//
void compute_positions()
{
  for (int i = 0; i < nbodies; i++)
    positions[i] = add_vectors(positions[i], add_vectors(velocities[i], scale_vector(0.5, accelerations[i])));
}

//
void simulate()
{
  compute_accelerations();
  compute_positions();
  compute_velocities();
  resolve_collisions();
}

//
int main(int argc, char **argv)
{
  //
  int i;
  unsigned char quit = 0;
  SDL_Event event;
  SDL_Window *window;
  SDL_Renderer *renderer;

  srand(time(NULL));
  
  //
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(800, 800, SDL_WINDOW_OPENGL, &window, &renderer);
  
  //
  init_system();
  
//Main loop
  
  FILE *imprimsc;
  imprimsc=fopen("out0_sd.dat","wt");
  for (int i = 0; !quit && i < timeSteps; i++)
    {	  
      //
      double before = (double)rdtsc();
      
      simulate();

      //
      double after = (double)rdtsc();
      
      //
      printf("%d %lf\n", i, (after - before));
      fprintf(imprimsc,"%d %f \n",i,(after - before));
      
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);
      
      for (int i = 0; i < nbodies; i++)
	{
	  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	  SDL_RenderDrawPoint(renderer, positions.x[i], positions.y[i]);
	}
      
      SDL_RenderPresent(renderer);
      
      SDL_Delay(10);
      
      while (SDL_PollEvent(&event))
	if (event.type == SDL_QUIT)
	  quit = 1;
	else
	  if (event.type == SDL_KEYDOWN)
	    if (event.key.keysym.sym == SDLK_q)
	      quit = 1;
    }
  fclose(imprimsc);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}
