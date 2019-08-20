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
float *matA;
float *matB;
float *matBt;
float *matAB;
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
    int offset1;
    int offset2;
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
    matA =(float*)  malloc(data.colA*data.rowA*sizeof(float));
    matB =(float*)  malloc(data.colB*data.rowB*sizeof(float));
    matBt =(float*)  malloc(data.colB*data.rowB*sizeof(float));
    matAB =(float*)  malloc(data.colB*data.rowA*sizeof(float));
    if(id == 0){
        fp = fopen(argv[1], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&data.rowA,&data.colA);
            for(i = 0 ; i < data.rowA ; i++){
                for(j = 0 ; j < data.colA ; j++){
                    fscanf(fp,"%f ",&num);
                    matA[i*data.colA+j] = num;
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
                    matB[i*data.colB+j] = num;
                }
            }
            fclose(fp);
        }
        for(i=0;i<data.rowB;i++){ //n = rowB
            for(j=0;j<data.colB;j++){ //p = colB
                matBt[i + j * data.rowB] = matB[i * data.colB + j];
            }
        }
    }
    MPI_Bcast(&matBt[0],data.rowB*data.colB,MPI_FLOAT,0,MPI_COMM_WORLD);
    if(id==0){
        startTime = MPI_Wtime();
        avgrow = data.rowA/p;
        extra = data.rowA%p;
        offset1 = avgrow*data.colA; //10 6 row 2 col 3
        offset2 = avgrow*data.colB;
        for(i=1;i<p;i++){
            rows = (i<=extra)?avgrow+1:avgrow;
            MPI_Send(&matA[offset1],    rows*data.colA,MPI_FLOAT,i,1,MPI_COMM_WORLD);
            offset1 += rows*data.colA;           
        }
        for(i=1;i<p;i++){
            rows = (i<=extra)?avgrow+1:avgrow;
            MPI_Recv(&matAB[offset2],   rows*data.colB,MPI_FLOAT,i,2,MPI_COMM_WORLD,&status);
            offset2 += rows*data.colB;
        }
        for ( i = 0; i < avgrow; ++i)
        {
            for ( j = 0; j < data.colB; ++j)
            {
                for ( k = 0; k < data.rowB; ++k)
                {
                    matAB[i * data.colB + j] += matA[i * data.rowB + k] * matBt[k + j * data.rowB];
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
                    fprintf(fp,"%.1f ",matAB[i*data.colB+j]);
                }
                fprintf(fp,"\n");
            }
            fclose(fp);
        }
    }
    else if(id!=0){
        avgrow = data.rowA/p;
        extra = data.rowA%p;
        rows = (id<=extra)?avgrow+1:avgrow;
        MPI_Recv(&matA[0], rows*data.colA,MPI_FLOAT,0,1,MPI_COMM_WORLD,&status);
        for ( i = 0; i < rows; ++i)
        {
            for ( j = 0; j < data.colB; ++j)
            {
                for ( k = 0; k < data.rowB; ++k)
                {
                     matAB[i*data.colB+j] += matA[i * data.rowB + k] * matBt[k + j * data.rowB];
                }
            }
        }
        MPI_Send(&matAB[0], rows*data.colB, MPI_FLOAT, 0, 2, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    freeall();
    return 0;
}
void freeall(){
    free(matA);
    free(matB);
    free(matAB);
    free(matBt);
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