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
int main(int argc,char** argv)
{
    int count,i,p,id,t;
    double num;
    double* data;
    double startTime;
    double endTime;
    FILE *fp;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id); 
    t = atoi(argv[3]);
    omp_set_num_threads(t);
    // fp = fopen(argv[1], "r");
    //     if(fp){
    //         fscanf(fp,"%d\n",&count);
    //         data = (double*) calloc(count,sizeof(double));
    //         for( i = 0; i < count; i++)
    //         {
    //             fscanf(fp,"%lf\n",&num);
    //             data[i] = num;
    //         }
    //         fclose(fp);
    //     }
    if(id ==0){
        fp = fopen(argv[1], "r");
        if(fp){
            fscanf(fp,"%d\n",&count);
            data = (double*) calloc(count,sizeof(double));
            for( i = 0; i < count; i++)
            {
                fscanf(fp,"%lf\n",&num);
                data[i] = num;
            }
            fclose(fp);
        }
    }
    // else{
    //     fp = fopen(argv[1], "r");
    //     if(fp){
    //         fscanf(fp,"%d\n",&count);
    //         data = (double*) calloc(count,sizeof(double));
    //         for( i = 0; i < count; i++)
    //         {
    //             fscanf(fp,"%lf\n",&num);
    //             data[i] = num;
    //         }
    //         fclose(fp);
    //     }
    // }
    MPI_Bcast(&count,    1,                      MPI_INT,    0,  MPI_COMM_WORLD);
    // if(id!=0){
    //     data = (double*) malloc(count*sizeof(double));
    // }
    // MPI_Bcast(&data,    count,                      MPI_DOUBLE,    0,  MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    startTime = MPI_Wtime();
    int* displs = (int *)malloc(p*sizeof(int)); 
    int* scounts = (int *)malloc(p*sizeof(int)); 
    int avg = count/p; //10/5 = 2
    int extra = count%p; //10%2 = 0
    int offset = 0;
    // printf("%d %d %d\n",id,avg,extra);
    for (i=0; i<p; i++) { 
        displs[i] = offset;
        scounts[i] = i==0?avg+extra:avg; 
        if(i==0) offset += avg+extra;
        else offset += avg;
    } 
    double *sub_data = (double *)malloc(sizeof(double) * scounts[id]);
    MPI_Scatterv(   data, scounts, displs, MPI_DOUBLE, 
                    sub_data, scounts[id], MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    if(id == 0){
        #pragma omp parallel num_threads(t)
        {
            #pragma omp single nowait
            {
                quicksort(sub_data,0,scounts[id]-1);
            }
        }
        printf("# %d: --> ",id);
        for(i=0;i<scounts[id];i++){
            printf("%.0lf ",sub_data[i]);
        }
        printf("\n");
    }
    else{
        // printf("---> %d %d %d \n",id,displs[id],scounts[id]);
        // printf("%d %.0lf %.0lf\n",id,data[displs[id]],sub_data[0]);
        // printf("%d %.0lf\n",id,sub_data[0]);
        #pragma omp parallel num_threads(t)
        {
            #pragma omp single nowait
            {
                quicksort(sub_data,0,scounts[id]-1);
            }
        }
        printf("# %d: --> ",id);
        for(i=0;i<scounts[id];i++){
            printf("%.0lf ",sub_data[i]);
        }
        printf("\n");
    }
    endTime = MPI_Wtime();
    MPI_Finalize();


}