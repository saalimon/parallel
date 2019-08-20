#include<stdio.h>
#include<math.h>
#include<stdlib.h>     /* srand, rand */
#include<time.h>
#include<cuda.h>
#include<curand_kernel.h>
#define grid_size 1
#define block_size 512
#define n 1000000
//rand() in gpu
__global__ void cal_pi(double *sum_D)
{
    double x,y;
    int i;
    int tid = blockIdx.x*blockDim.x+threadIdx.x;
    int points_in_circle = 0;
    

    curandState_t rng;
    curand_init(clock64(),tid,0,&rng);

    for(i=0;i<n;i++){
        x = curand_uniform(&rng);
        y = curand_uniform(&rng);
        points_in_circle += (x*x + y*y <= 1.0f);
    }
    sum_D[tid] += 4.0f * points_in_circle / (float) n;
}
double uniform(double a, double b){
    return rand() / (RAND_MAX + 1.0) * (b - a) + a;
}
double cpu_monte_carlo(long n){
	double x, y;
	long in_circle = 0,i;
	double r = 5.0;
	float a = -r,b = r;
	for(i = 0; i < n; i++) {
		x = uniform(a,b);
		y = uniform(a,b);
		in_circle += x*x + y*y <= r*r ? 1 : 0;
	}
	return ((double) in_circle / n * 4);
}
int main(int argc, char **argv){
    int i;
    double pi_gpu,size;
    double *sum_h, *sum_D;
    clock_t start_gpu, end_gpu;
    clock_t start_cpu, end_cpu;
    
    size = grid_size*block_size*sizeof(double);
    sum_h = (double *)malloc(size);
    //start run in cpu
    start_cpu = clock();
    double pi_cpu = cpu_monte_carlo(block_size * grid_size * n);
    end_cpu = clock();
    printf("Pi by CPU = %lf from ramdom %d dots\n",pi_cpu,n);
    printf("Time: %f sec.\n",(end_cpu - start_cpu)/(float)CLOCKS_PER_SEC);

    //start run in gpu
    start_gpu = clock();
    cudaMalloc((void **)&sum_D, size);
    cal_pi<<<block_size, grid_size>>>(sum_D);
    cudaMemcpy(sum_h, sum_D, size,cudaMemcpyDeviceToHost);
    pi_gpu = 0;
    for(i = 0; i< grid_size*block_size;i++){
        pi_gpu += sum_h[i];
    }
    pi_gpu /=(block_size*grid_size);
    end_gpu = clock();
    
    printf("Pi by GPU = %lf from ramdom %d dots\n",pi_gpu,n);
    printf("Time: %f sec.\n",(end_gpu - start_gpu)/(float)CLOCKS_PER_SEC);
    cudaFree(sum_D);
    free(sum_h);

    return 0;
}