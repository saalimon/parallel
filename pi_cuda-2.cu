#include <stdlib.h>
#include <stdio.h>
#include <cuda.h>
#include <math.h>
#include <time.h>
#include <curand_kernel.h>

#define TRIALS_PER_THREAD 10000
#define BLOCKS 512
#define THREADS 1
#define PI 3.1415926535  // known value of pi
double uniform(double a, double b){
    return rand() / (RAND_MAX + 1.0) * (b - a) + a;
}
__global__ void gpu_monte_carlo(float *estimate, curandState *states) {
	unsigned int tid = threadIdx.x + blockDim.x * blockIdx.x;
	int points_in_circle = 0;
	float x, y;

	curand_init(1234, tid, 0, &states[tid]);  // 	Initialize CURAND


	for(int i = 0; i < TRIALS_PER_THREAD; i++) {
		x = curand_uniform (&states[tid]);
		y = curand_uniform (&states[tid]);
		points_in_circle += (x*x + y*y <= 1.0f); // count if x & y is in the circle.
	}
	estimate[tid] = 4.0f * points_in_circle / (float) TRIALS_PER_THREAD; // return estimate of pi
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
	return ((double)in_circle/n)*4;
}
int main (int argc, char *argv[]) {
	clock_t startgpu, stopgpu,startcpu,stopcpu;
	float host[BLOCKS * THREADS];
	float *dev;
	curandState *devStates;

	startcpu = clock();
	float pi_cpu = cpu_monte_carlo(BLOCKS * THREADS * TRIALS_PER_THREAD);
	stopcpu = clock();
	printf("Pi = %f	CPU pi calculated in %f s.\n", pi_cpu,(stopcpu-startcpu)/(float)CLOCKS_PER_SEC);


	startgpu = clock();

	cudaMalloc((void **) &dev, BLOCKS * THREADS * sizeof(float)); // allocate device mem. for counts
	
	cudaMalloc( (void **)&devStates, THREADS * BLOCKS * sizeof(curandState) );

	gpu_monte_carlo<<<BLOCKS, THREADS>>>(dev, devStates);

	cudaMemcpy(host, dev, BLOCKS * THREADS * sizeof(float), cudaMemcpyDeviceToHost); // return results 

	float pi_gpu;
	for(int i = 0; i < BLOCKS * THREADS; i++) {
		pi_gpu += host[i];
	}

	pi_gpu /= (BLOCKS * THREADS);

	stopgpu = clock();
	printf("Pi = %f	GPU pi calculated in %f s.\n", pi_gpu,(stopgpu-startgpu)/(float)CLOCKS_PER_SEC);

	
	return 0;
}