#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<omp.h> 
void quicksort(double * data, int first, int last)
{
    int partition, i;
    double temp;
    double pivot;
    if (last <= 1)
        return;
    pivot = data[first + last/2];
    temp = data[first];
    data[first] = data[first + last/2];
    data[first + last/2] = temp;
    partition = first;
    for (i = first+1; i < first+last; i++)
        if (data[i] < pivot) {
        partition++;
        temp = data[i];
        data[i] = data[partition];
        data[partition] = temp;
        }
    temp = data[first];
    data[first] = data[partition];
    data[partition] = temp;
    #pragma omp task
    quicksort(data, first, partition-first);
    #pragma omp task
    quicksort(data, partition+1, first+last-partition-1);
}
double * merge(double * v1, int n1, double * v2, int n2)
{
  double * result = (double *)malloc((n1 + n2) * sizeof(double));
  int i = 0;
  int j = 0;
  int k;
  for (k = 0; k < n1 + n2; k++) {
    if (i >= n1) {
      result[k] = v2[j];
      j++;
    }
    else if (j >= n2) {
      result[k] = v1[i];
      i++;
    }
    else if (v1[i] < v2[j]) { 
      result[k] = v1[i];
      i++;
    }
    else { 
      result[k] = v2[j];
      j++;
    }
  }
  return result;
}
int main(int argc,char** argv)
{
    int count,i,p,id,t;
    double num;
    double* data;
    double startTime;
    double endTime;
    double parallel_time_i,parallel_time_e;

    double *sub;
    int id_send;
    FILE *fp;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id); 
    t = atoi(argv[3]);
    omp_set_num_threads(t);
    startTime = MPI_Wtime();
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
    parallel_time_i = MPI_Wtime();
    MPI_Bcast(&count,    1,                      MPI_INT,    0,  MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    int* displs = (int *)malloc(p*sizeof(int)); 
    int* scounts = (int *)malloc(p*sizeof(int)); 
    int extra = count%p; //10%2 = 0
    int avg = extra != 0 ? count/p + 1: count/p; //10/5 = 2
    int offset = 0;
    double *sub_data = (double *)malloc(sizeof(double) * avg);
    MPI_Scatter(   data, avg, MPI_DOUBLE, sub_data, avg, MPI_DOUBLE, 0, MPI_COMM_WORLD  );
    int s = (count >= avg * (id+1)) ? avg : count - avg * id;
    #pragma omp parallel num_threads(t)
        {
            #pragma omp single nowait
            {
                quicksort(sub_data,0,s);
            }
        }
    // pte_quick = MPI_Wtime();
    for(i=1;i<p;i=2*i){
        if(id%(2*i)!=0){ //id is odd process then send to even
            MPI_Send(sub_data,s,MPI_DOUBLE,id-i,0,MPI_COMM_WORLD);
            break;
        }
        if(id+i<p){
            int o = (count >= avg * (id+2*i)) ? avg * i : count - avg * (id+i);
            double *sub_other = (double *)malloc(sizeof(double) * o);  
            MPI_Recv(sub_other,o,MPI_DOUBLE,id+i,0,MPI_COMM_WORLD,&status);
            #pragma omp parallel num_threads(t)
            {
                data = merge(sub_data,s,sub_other,o);
            }
            free(sub_data);
            free(sub_other);
            sub_data = data;
            s = s+o;
        }
    }
    // parallel_time_e = MPI_Wtime();
    if (id==0) {
        endTime = MPI_Wtime();
        fp = fopen(argv[2], "w");
        if (fp) {
            fprintf(fp,"%d\n",count);
            for(i = 0; i < count ; i++){
                fprintf(fp,"%.1lf\n",sub_data[i]);  
            }
            fclose(fp);
        }
        printf("Parallel Time:   %f\n",endTime-parallel_time_i);
        printf("TIME:   %f\n",endTime-startTime);
    }
    free(data);
    data = NULL;
    MPI_Finalize();
}