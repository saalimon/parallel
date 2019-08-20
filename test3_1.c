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
        slave_loc = slave_data +  (total-(slave_data*p));
        matAB = malloc(nrow*ncol*sizeof(double));
        startTime = MPI_Wtime();
        // printf("%d %d %d\n",slave_data,p,slave_loc);
        for(int j = 0 ;j < slave_loc ;j++){
            matAB[j] = matA[j]+matB[j];
        }
        for(i = 1 ;i <p ; i++){
            MPI_Send(&slave_loc,    1, MPI_INT,     i, 1, MPI_COMM_WORLD);
            MPI_Send(&slave_data,   1, MPI_INT,     i, 2, MPI_COMM_WORLD);
        }
        for(i = 1; i < p ; i++){
            for(j = slave_loc*i ; j<(slave_loc*i)+slave_data ; j++){
                MPI_Send(&matA[j],    1, MPI_DOUBLE,   i, 3, MPI_COMM_WORLD); 
                MPI_Send(&matB[j],    1, MPI_DOUBLE,   i, 4, MPI_COMM_WORLD); 
                MPI_Recv(&matAB[j],    1, MPI_DOUBLE,   i, 5, MPI_COMM_WORLD,&status);
            }
        }
        //  for(i = 1; i < p ; i++){
        //     for(j = slave_loc*i ; j<(slave_loc*i)+slave_data ; j++){
        //         MPI_Send(&matB[j],    1, MPI_DOUBLE,   i, 4, MPI_COMM_WORLD); 
        //     }
        // }
        // for(i = 1; i < p ; i++){
        //     for(j = slave_loc*i ; j<(slave_loc*i)+slave_data ; j++){
        //         MPI_Recv(&matAB[j],    1, MPI_DOUBLE,   i, 5, MPI_COMM_WORLD,&status);
        //     }
        // }
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
        MPI_Recv(&slave_loc,     1, MPI_DOUBLE,     0, 1, MPI_COMM_WORLD,&status);
        MPI_Recv(&slave_data,    1, MPI_DOUBLE,     0, 2, MPI_COMM_WORLD, &status);
        for(j = slave_loc*id ; j<(slave_loc*id)+slave_data ; j++){
                double matA_recv ;
                double matB_recv ;
                double matAB_recv ;
                MPI_Recv(&matA_recv,    1, MPI_DOUBLE,   0, 3, MPI_COMM_WORLD,&status);
                MPI_Recv(&matB_recv,    1, MPI_DOUBLE,   0, 4, MPI_COMM_WORLD,&status);
                matAB_recv = matA_recv+matB_recv;
                MPI_Send(&matAB_recv,   1, MPI_DOUBLE,   0, 5, MPI_COMM_WORLD); 
        }
        // MPI_Send(&matAB_recv,   1, MPI_DOUBLE,   0, 3, MPI_COMM_WORLD);
        
   }
   MPI_Finalize();
    return 0;
}