#include <stdlib.h>
#include <stdio.h>
#include <cuda.h>
#include <math.h>
#include <time.h>
#include <curand_kernel.h>
#define ROUNDS 1000000
#define BLOCKS 512
#define GRIDS 1
double uniform(double a, double b){
    return rand() / (RAND_MAX + 1.0) * (b - a) + a;
}
__global__ void gpu_monte_carlo(float *pi, curandState *states) {
	unsigned int t_rank = threadIdx.x + blockDim.x * blockIdx.x;
	long in_circle = 0;
	float x, y;

	curand_init(1234, t_rank, 0, &states[t_rank]);  // 	Initialize CURAND


	for(long i = 0; i < ROUNDS; i++) {
		x = curand_uniform (&states[t_rank]);
		y = curand_uniform (&states[t_rank]);
		in_circle += x*x + y*y <= 1.0f ? 1 : 0; // count if x & y is in the circle.
	}
	pi[t_rank] = (float) in_circle / ROUNDS * 4.0; // return estimate of pi
}
double cpu_monte_carlo(long n) {
	double x, y;
	long in_circle;
	double r = 5.0;
	float a = -r,b = r;
	for(long i = 0; i < n; i++) {
		x = uniform(a,b);
		y = uniform(a,b);
		in_circle += x*x + y*y <= r*r ? 1 : 0;
	}
	return (double) in_circle / n * 4;
}
int main (int argc, char *argv[]) {
	clock_t startgpu, stopgpu,startcpu,stopcpu;
	float host[BLOCKS * GRIDS];
	float *dev;
	curandState *devStates;

	startcpu = clock();
	float pi_cpu = cpu_monte_carlo(BLOCKS * GRIDS * ROUNDS);
	stopcpu = clock();
	printf("Pi = %f	CPU pi calculated in %f s.\n", pi_cpu,(stopcpu-startcpu)/(float)CLOCKS_PER_SEC);


	startgpu = clock();

	cudaMalloc((void **) &dev, BLOCKS * GRIDS * sizeof(float)); // allocate device mem. for counts
	
	cudaMalloc( (void **)&devStates, GRIDS * BLOCKS * sizeof(curandState) );

	gpu_monte_carlo<<<BLOCKS, GRIDS>>>(dev,devStates);

	cudaMemcpy(host, dev, BLOCKS * GRIDS * sizeof(float), cudaMemcpyDeviceToHost); // return results 

	float pi_gpu;

	for(int i = 0; i < BLOCKS * GRIDS; i++) {
		pi_gpu += host[i];
	}

	pi_gpu /= (BLOCKS * GRIDS);

	stopgpu = clock();
	printf("Pi = %f	GPU pi calculated in %f s.\n", pi_gpu,(stopgpu-startgpu)/(float)CLOCKS_PER_SEC);

	
	return 0;
}