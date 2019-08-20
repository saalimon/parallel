#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
int main(int argc,char** argv)
{
    int i,j;
    int c;
    int id;
    int p;
    int total;
    int size_mat;
    char buff[1024];
    char endline;
    int nrow,ncol;
    int slave_loc;
    int slave_data;
    int start_loc;
    int chucksize;
    double num;
    double *matA;
    double *matB;
    double *matAB;
    double startTime;
    double endTime;
    FILE *fp;
    MPI_Request request[6];
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);     
    if(id == 0){    //read file of file 1
        i = 0;
        j = 1;
        fp = fopen(argv[1], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&nrow,&ncol);
            matA = malloc(nrow*ncol*sizeof(double));
            while ((c = fscanf(fp,"%lf ",&num)) !=EOF){
                matA[i++] = num;
            }
            fclose(fp);

        }
        i = 0;
        fp = fopen(argv[2], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&nrow,&ncol);
            matB = malloc(nrow*ncol*sizeof(double));
            while ((c = fscanf(fp,"%lf ",&num)) !=EOF){
                matB[i++] = num;
            }
            fclose(fp);
        }
        total = nrow*ncol;
        slave_data = total/p;
        slave_loc = (total-(slave_data*p));
        matAB = malloc(nrow*ncol*sizeof(double));
        startTime = MPI_Wtime();
        for(int j = 0 ;j < slave_data+slave_loc ;j++){
            matAB[j] = matA[j]+matB[j];
        }
        chucksize = slave_data;
        for(i = 1 ;i <p ; i++){
            MPI_Send(&slave_data,   1, MPI_INT,     i, 1, MPI_COMM_WORLD);
            MPI_Send(&matA[i*slave_data+slave_loc],    slave_data, MPI_DOUBLE,   i, 2, MPI_COMM_WORLD); 
            MPI_Send(&matB[i*slave_data+slave_loc],    slave_data, MPI_DOUBLE,   i, 3, MPI_COMM_WORLD); 
            MPI_Recv(&matAB[i*slave_data+slave_loc],    slave_data, MPI_DOUBLE,   i, 4, MPI_COMM_WORLD,&status); 
        }
        endTime = MPI_Wtime();
        fp = fopen(argv[3], "w");
        if (fp) {
            fprintf(fp,"%d %d\n",nrow,ncol);
            for(i = 0;i<total;i++){
                    fprintf(fp,"%.7lf ",matAB[i]);
                if((i+1)%ncol==0)
                    fprintf(fp,"\n");
            }
            
        }
        fclose(fp);
        printf("Timings :%f Sec\n",endTime-startTime);
        free(matA);
        free(matB);
        free(matAB);
    }
    else {  
        MPI_Recv(&slave_data,    1, MPI_INT,     0, 1, MPI_COMM_WORLD, &status);
        double *matA_recv = malloc(slave_data*sizeof(double));
        double *matB_recv = malloc(slave_data*sizeof(double));
        double *matAB_recv = malloc(slave_data*sizeof(double));
        MPI_Recv(matA_recv,  slave_data, MPI_DOUBLE,     0, 2, MPI_COMM_WORLD, &status);
        MPI_Recv(matB_recv,  slave_data, MPI_DOUBLE,     0, 3, MPI_COMM_WORLD, &status);
        for(i=0;i<slave_data;i++){
            matAB_recv[i] = matA_recv[i]+matB_recv[i];
        }
        MPI_Send(matAB_recv,    slave_data, MPI_DOUBLE,   0, 4, MPI_COMM_WORLD);   
   }
   MPI_Finalize();
    return 0;
}