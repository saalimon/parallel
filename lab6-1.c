#include <omp.h> 
#include<stdio.h>
int main (int argc, char *argv[]) { 
int nt, rank; 
    // omp_set_num_threads 
    omp_set_num_threads(3);
    #pragma omp parallel private(nt, rank) 
    { 
        rank = omp_get_thread_num(); 
        printf("Hello World from thread = %d\n", rank); 

        if (rank == 0) { 
            nt = omp_get_num_threads(); 
            printf("Number of threads = %d\n", nt); 
        } 
    } 
} 