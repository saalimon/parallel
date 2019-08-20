#include <stdio.h>

__global__ void matAdd(double *a_D, double *c_D)
{
   int t_rank;

   t_rank = threadIdx.y*blockDim.x + threadIdx.x;
   c_D[t_rank] = t_rank+1;
}

int main(int argc, char **argv)
{
   int i, j;
   int size, block_size = 8, grid_size = 1;
   double *c_H, *c_D, *a_H, *a_D;
   
   size = block_size*block_size*sizeof(double);
   a_H = (double *)malloc(size);
   c_H = (double *)malloc(size);
   cudaMalloc((void **)&a_D, size);
   cudaMalloc((void **)&c_D, size);
   
   for (i=0; i<block_size; i++)
      for (j=0; j<block_size; j++)
         a_H[(i*block_size)+j]=(i*block_size)+j;
   for(i = 0; i < block_size; i++, printf("\n"))
      for (j=0; j < block_size; j++) 
         //printf("matric[%d][%d] is %f\n", i, j, c_H[(i*block_size)+j]);
         printf(" %4.1f", a_H[(i*block_size)+j]);
   printf("\n");
   cudaMemcpy(a_D, a_H, size, cudaMemcpyHostToDevice);

   dim3 Block(block_size, block_size);
   dim3 Grid(grid_size, grid_size);

   matAdd<<<Grid, Block>>>(a_D, c_D);

   cudaMemcpy(c_H, c_D, size, cudaMemcpyDeviceToHost);

   for(i = 0; i < block_size; i++, printf("\n"))
      for (j=0; j < block_size; j++) 
         //printf("matric[%d][%d] is %f\n", i, j, c_H[(i*block_size)+j]);
         printf(" %4.1f", c_H[(i*block_size)+j]);

   free(a_H);
   free(c_H);
   cudaFree(a_D);
   cudaFree(c_D);

   return 0;
}

