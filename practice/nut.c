#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc,char** argv){

int p,id;
int nrow,ncol;
int part,frac;
double *matA,*matB,*result,*result_temp,*matA_recv,*matB_recv;
int n,c,i,j,a,total;
int start;
double num;
double temp;
FILE *fp;
MPI_Status status;

    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id); 

    if(id == 0){
        result = malloc(nrow*ncol*sizeof(double));
        i = 0;
        fp = fopen(argv[1], "r");
        if (fp) {
            fscanf(fp,"%d\n",&nrow);
            matA = malloc(nrow*ncol*sizeof(double));
            while ((c = fscanf(fp,"%lf ",&num)) !=EOF){
                matA[i++] = num;
            }
            fclose(fp);

        }
        i = 0;
        fp = fopen(argv[2], "r");
        if (fp) {
            fscanf(fp,"%d\n",&nrow);
            matB = malloc(nrow*ncol*sizeof(double));
            while ((c = fscanf(fp,"%lf ",&num)) !=EOF){
                matB[i++] = num;
            }
            fclose(fp);
        }
    part = nrow/p;
    total = nrow*ncol;
    printf("Read OK\n");
        if(p>1){

            for(i=1;i<p;i++){
                start = part*i;
                MPI_Send(&total,    1, MPI_INT,     i, 0, MPI_COMM_WORLD);
                MPI_Send(&part,    1, MPI_INT,     i, 1, MPI_COMM_WORLD);
                MPI_Send(&start,    1, MPI_INT,     i, 2, MPI_COMM_WORLD);
                MPI_Send(matA,    nrow*ncol, MPI_DOUBLE,     i, 3, MPI_COMM_WORLD);
                MPI_Send(matB,    nrow*ncol, MPI_DOUBLE,     i, 4, MPI_COMM_WORLD);
            }   

            for(i=0;i<part;i++){
                result[i] = matA[i] + matB[i];
            }

            for(i=1;i<p;i++){
                MPI_Recv(&result[part*i],    part, MPI_DOUBLE,   i, 5, MPI_COMM_WORLD, &status);
            }

            for(i=0;i<ncol*nrow;i++){
                printf("%.1lf ",result[i]);
            }

        }

        else{
            for(i=0;i<nrow*ncol;i++){
                result[i] = matA[i] + matB[i];
            }
            for(i=0;i<ncol*nrow;i++){
                printf("%.1lf ",result[i]);
            }
        }
    }
    else{
        matA = malloc(nrow*ncol*sizeof(double));
        matB = malloc(nrow*ncol*sizeof(double));
        result_temp = malloc(nrow*ncol*sizeof(double));

        MPI_Recv(&total,    1, MPI_INT,   0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&part,    1, MPI_DOUBLE,   0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&start,    1, MPI_DOUBLE,   0, 2, MPI_COMM_WORLD, &status);
        MPI_Recv(&matA_recv,   total, MPI_DOUBLE,   0, 3, MPI_COMM_WORLD, &status);
        MPI_Recv(&matB_recv,   total, MPI_DOUBLE,   0, 4, MPI_COMM_WORLD, &status);

        for(i=start;i<id*part;i++){
            result_temp[i] = matA_recv[i] + matB_recv[i];
        }
        MPI_Send(&result_temp,    part, MPI_DOUBLE,     0, 5, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}