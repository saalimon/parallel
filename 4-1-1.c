#include<stdio.h>
#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include <stddef.h> //declare for offsetof
typedef struct rowcol_s {
    int rowA;
    int colA;
    int rowB;
    int colB;
} rc;
void freeall();
float **matA;
float **matB;
float **matBt;
float **matAB;
float **submatA;
int *displs;
int *scounts;
int main(int argc,char** argv)
{
    int i,j,k;
    int c;
    int id;
    int p;
    int pos = 0;
    /*THIS PART IS VARIABLE THAT USED FOR CALCULATION*/
    int avgrow;
    int extra;
    int offset;
    int rows;
    /*END PART*/
    float num;
    float sum = 0;
    double startTime;
    double endTime;
    char msg[200];
    FILE *fp;
    MPI_Request request[6];
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);  

    /*CREATE STRUCT*/
    const int nitems=4;
    int          blocklengths[4] = {1,1,1,1};
    MPI_Datatype types[4] = {MPI_INT, MPI_INT,MPI_INT,MPI_INT};
    MPI_Datatype mpi_rc_type;
    MPI_Aint     offsets[4];

    offsets[0] = offsetof(rc, rowA);
    offsets[1] = offsetof(rc, colA);
    offsets[2] = offsetof(rc, rowB);
    offsets[3] = offsetof(rc, colB);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_rc_type);
    MPI_Type_commit(&mpi_rc_type);
    /*END*/
    rc data; //declare rc struct as data
    if(id == 0){
        fp = fopen(argv[1], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&data.rowA,&data.colA);          
            fclose(fp);
        }
        fp = fopen(argv[2], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&data.rowB,&data.colB);
            fclose(fp);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&data,    1,                      mpi_rc_type,    0,  MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    matA =(float**)  malloc(data.rowA*sizeof(float*));
    for(i = 0 ; i < data.rowA ; i++){
        matA[i] = (float*) malloc(data.colA*sizeof(float));
    }
    matB =(float**)  malloc(data.rowB*sizeof(float*));
    for(i = 0 ; i < data.rowB ; i++){
        matB[i] = (float*) malloc(data.colB*sizeof(float));
    }
    matBt =(float**)  malloc(data.colB*sizeof(float*));
    for(i = 0 ; i < data.colB ; i++){
        matBt[i] = (float*) malloc(data.rowB*sizeof(float));
    }
    matAB =(float**)  malloc(data.rowA*sizeof(float*));
    for(i = 0 ; i < data.rowA ; i++){
        matAB[i] = (float*) malloc(data.colB*sizeof(float));
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(id == 0){
        fp = fopen(argv[1], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&data.rowA,&data.colA);
            for(i = 0 ; i < data.rowA ; i++){
                for(j = 0 ; j < data.colA ; j++){
                    fscanf(fp,"%f ",&num);
                    matA[i][j] = num;
                }
            }            
            fclose(fp);
        }
        fp = fopen(argv[2], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&data.rowB,&data.colB);
            for(i = 0; i < data.rowB ; i++){
                for(j = 0 ; j < data.colB ; j++){
                    fscanf(fp,"%f ",&num);
                    matB[i][j] = num;
                }
            }
            fclose(fp);
        }
        for(i=0;i<data.rowB;i++){ //n = rowB
            for(j=0;j<data.colB;j++){ //p = colB
                matBt[j][i] = matB[i][j];
            }
        }
    }
    MPI_Bcast(&matBt[0][0],data.colB*data.rowB,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if(id==0){
        startTime = MPI_Wtime();
        avgrow = data.rowA/p;
        extra = data.rowA%p;
        offset = 0;
        offset =avgrow+extra;
        // printf("%d %d %d\n",0,avgrow,offset);
        for(i=1;i<p;i++){
            MPI_Send(&matA[offset][0], avgrow*data.colA,MPI_FLOAT,i,3,MPI_COMM_WORLD);
            //printf("%d %d\n",avgrow*data.colA,offset);
            offset+=avgrow;
            //printf("%d %d %d\n",i,avgrow,offset);
        }
        offset =avgrow+extra;
        for(i=1;i<p;i++){
            MPI_Recv(&matAB[offset][0], avgrow*data.colB,MPI_FLOAT,i,6,MPI_COMM_WORLD,&status);
            //printf("%d %d %d\n",i,avgrow,offset);
            offset+=avgrow;
        }
        printf("%d %d\n",data.colA,data.rowB);
        for ( i = 0; i < (avgrow+extra); i++)
        {
            for ( j = 0; j < data.colB; j++)
            {
                matAB[i][j] = 0;
                for ( k = 0; k < data.rowB; k++)
                {
                    matAB[i][j] += matA[i][k] * matBt[j][k];
                } 
            }
        }
        endTime = MPI_Wtime();
        printf("Timings :%lf Sec\n",endTime-startTime);
        fp = fopen(argv[3], "w");
        if (fp) {
            fprintf(fp,"%d %d\n",data.rowA,data.colB);
            for(i = 0 ; i < data.rowA ; i++){
                for(j = 0 ; j< data.colB ; j++){
                    fprintf(fp,"%.1f ",matAB[i][j]);
                }
                fprintf(fp,"\n");
            }
            fclose(fp);
        }
    }
    else{
        avgrow = data.rowA/p;
        extra = data.rowA%p;
        offset = 0;
        float **matA_re = (float**)  malloc(avgrow*sizeof(float*));
        for(i = 0 ; i < avgrow ; i++){
            matA_re[i] = (float*) malloc(data.colA*sizeof(float));
        }
        //printf("%d %d %d %d\n",avgrow,data.colA,data.colB,data.rowB);
        MPI_Recv(&matA_re[0][0], avgrow*data.colA,MPI_FLOAT,0,3,MPI_COMM_WORLD,&status);
        for ( i = 0; i < avgrow; i++)
        {
            for ( j = 0; j < data.colB; j++)
            {
                matAB[i][j] = 0;
                for ( k = 0; k < data.rowB; k++)
                {
                    matAB[i][j] += matA_re[i][k] * matBt[j][k];
                } 
            }
        }
        // for(j=0;j<avgrow;j++){
        //         for(k=0;k<data.colA;k++){
        //             printf("%.1f ",matA_re[j][k]);
        //         }   
        //         printf("\n");
        //     }
        MPI_Send(&matAB[0][0], avgrow*data.colB, MPI_FLOAT, 0, 6, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    freeall();
    return 0;
}
void freeall(){
    free(matA);
    free(matB);
    free(matAB);
    free(submatA);
    free(displs);
    free(scounts);
}
    // int data1[2][3] = {{1,2,3},{4,5,6}}; // rA = 2 c = 3 <--
    // int data2[3][2] = {{7,8},{9,10},{11,12}}; // r = 3 <--  cB = 2
    // int data3[2][2];
    // for( i = 0; i<rowA;i++){
    //     for(k=0;k<colB;k++){
    //         sum = 0;
    //         for(j=0;j<colA;j++){
    //             sum += data1[i][j]*data2[j][k];
    //         }
    //         data3[i][k] = sum;
    //     }            
    // }
    // for(i = 0;i<rowA;i++){
    //     for(j=0;j<colB;j++){
    //         printf("%d ",data3[i][j]);
    //     }
    //     printf("\n");
    // }