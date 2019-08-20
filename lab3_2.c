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
    int start_loc;
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
    // size_mat = 10100;
    // matA = malloc(size_mat);
    // matB = malloc(size_mat);
    // matAB = malloc(size_mat);
    if(id == 0){    //read file of file 1
        i = 0;
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
        matAB = malloc(nrow*ncol*sizeof(double));
        slave_loc = total/p;
        slave_loc += total-(slave_loc*p);
        startTime = MPI_Wtime();
        for(int j = 0 ;j < slave_loc ;j++){
            matAB[j] = matA[j]+matB[j];
        }
  
        for(i = 1; i < p;i++){
            MPI_Send(&total,    1, MPI_INT,     i, 1, MPI_COMM_WORLD);
            MPI_Send(&slave_loc,1, MPI_INT,     i, 2, MPI_COMM_WORLD);
            MPI_Send(matA,    total, MPI_DOUBLE,   i, 3, MPI_COMM_WORLD);
            MPI_Send(matB,    total, MPI_DOUBLE,   i, 4, MPI_COMM_WORLD);
            MPI_Send(matAB,   total, MPI_DOUBLE,   i, 5, MPI_COMM_WORLD);
            MPI_Irecv(matAB,   total, MPI_DOUBLE,   i, 6, MPI_COMM_WORLD, &request[5]);
            MPI_Wait(&request[5],&status);
        }
        //  for(i = 0;i<total;i++){
        //     printf("%.7lf ",matAB[i]);
        //     if((i+1)%ncol==0){
        //         printf("\n");
        //     }
        //  }
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
                //MPI_Send(c,sizeof(double),MPI_FLOAT,1,1,MPI_COMM_WORLD);
        fclose(fp);
        printf("Timings :%f Sec\n",endTime-startTime);
        free(matA);
        free(matB);
        free(matAB);
    }
    else {
        int total_slave;
        int loc_slave;
        MPI_Irecv(&total_slave,    1, MPI_INT,     0, 1, MPI_COMM_WORLD,&request[0]);
        MPI_Wait(&request[0],&status);
        //printf("(%d) %d\n\n",id,total_slave);
        double *matA_recv = malloc(total_slave*sizeof(double));
        double *matB_recv = malloc(total_slave*sizeof(double));
        double *matAB_recv = malloc(total_slave*sizeof(double));
        MPI_Irecv(&loc_slave,1, MPI_INT,     0, 2, MPI_COMM_WORLD,&request[1]);
        MPI_Irecv(matA_recv,    total_slave, MPI_DOUBLE,   0, 3, MPI_COMM_WORLD, &request[2]);
        MPI_Irecv(matB_recv,    total_slave, MPI_DOUBLE,   0, 4, MPI_COMM_WORLD, &request[3]);
        MPI_Irecv(matAB_recv,   total_slave, MPI_DOUBLE,   0, 5, MPI_COMM_WORLD, &request[4]);
        MPI_Wait(&request[1],&status);
        MPI_Wait(&request[2],&status);
        MPI_Wait(&request[3],&status);
        MPI_Wait(&request[4],&status);
        for(int j = loc_slave*id ; j < (loc_slave*id)+loc_slave ;j++){
            matAB_recv[j] = matA_recv[j]+matB_recv[j];
            //printf("id --> (%d)   %.7lf ",id,matA_recv[j]);
        }
        MPI_Send(matAB_recv,   total_slave, MPI_DOUBLE,   0, 6, MPI_COMM_WORLD);
        free(matA_recv);
        free(matB_recv);
        free(matAB_recv);
   }
//    free(matA);
//    free(matB);
//    free(matAB);
   MPI_Finalize();
    return 0;
}
