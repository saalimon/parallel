#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<omp.h> 

void quicksort(double *matC,int first,int last){
    int i,j,pivot;
    double temp;
    if(first<last){
        pivot = first;
        i = first;
        j = last;
        while(i<j){
            while(matC[i]<=matC[pivot]&&i<last){
                i++;
            }
            while(matC[j]>matC[pivot]){
                j--;
            }
            if(i<j){
                temp = matC[i];
                matC[i] = matC[j];
                matC[j] = temp; 
            }
        }
        temp = matC[pivot];
        matC[pivot] = matC[j];
        matC[j] = temp;
        //#pragma omp task shared(matC) firstprivate(first,j)
        #pragma omp task 
        quicksort(matC,first,j-1);
        //#pragma omp task shared(matC) firstprivate(last,j)
        #pragma omp task 
        quicksort(matC,j+1,last); 
    }
}

int main(int argc,char** argv){
    int count,i,p,id;
    int t;
    double num;
    double* matA;
    double* matB;
    double startTime;
    double endTime;
    FILE *fp;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id); 
    t = atoi(argv[3]);
    omp_set_num_threads(t);
    if(id ==0){
        fp = fopen(argv[1], "r");
        if(fp){
            fscanf(fp,"%d\n",&count);
            matA = (double*) malloc(count*sizeof(double));
            for( i = 0; i < count; i++)
            {
                fscanf(fp,"%lf\n",&num);
                matA[i] = num;
            }
            fclose(fp);
        }
   }
    
    int pcount = count/p;
    int ex = count - pcount*p;
    startTime = MPI_Wtime();
    if (id==0) {
        #pragma omp parallel num_threads(t)
        {
            #pragma omp single nowait
            {
                quicksort(matA,0,count-1);
            }
        }
        endTime = MPI_Wtime();
        fp = fopen(argv[2], "w");
        if (fp) {
            fprintf(fp,"%d\n",count);
            for(i = 0; i < count ; i++){
                fprintf(fp,"%.1lf\n",matA[i]);  
            }
            fclose(fp);
        }
        printf("TIME:   %f\n",endTime-startTime);

    }
    MPI_Finalize();
    // free(matA);
    // free(matB);
    return 0;
}