#include<stdio.h>
#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<math.h>
int main(int argc,char** argv)
{
    int id;
    int p;
    int i,j,k;
    int row,col;
    int offsetr,offsetc,rtemp,ctemp;
    int count;
    int avgrow;
    int avgcol;
    int extrarow;
    int extracol;
    float **s;
    float **snew;
    float diffnorm = 0;
    double startTime;
    double endTime;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    // initial in matrix
    startTime = MPI_Wtime();
    if(id == 0){
        printf("Enter number of row and colunm : ");
        scanf("%d %d",&row,&col);
        printf("Enter number of round : ");
        scanf("%d",&count);
        s = (float**) malloc((row)*sizeof(float*));
        for(i=0;i<row;i++)
            s[i] = (float*) malloc((col)*sizeof(float));
        for(i=0; i< row;i++){
            for(j = 0; j < col; j++){
                if( i==0 || j==0 || i == row-1 || j == col -1)
                    s[i][j] = 255;
                else
                    s[i][j] = 0;
            }
        }
        snew = (float**) malloc((row)*sizeof(float*));
        for(i=0;i<row;i++)
            snew[i] = (float*) malloc(col*sizeof(float));
    }
    MPI_Bcast(&s,1,MPI_FLOAT,0,MPI_COMM_WORLD);
    avgrow = row/p;
    avgcol = col/p;
    extrarow = row - (avgrow*p);
    extracol = col - (avgcol*p);
    offsetr = 0;
    offsetr += avgrow;
    offsetc = 0;
    offsetc += avgcol;
    for(k=0;k<count;k++){
        for(i = 1; i < p; i++)
        {
            rtemp = (i<= extrarow)? avgrow+1:avgrow;
            ctemp = (i<= extracol)? avgcol+1:avgcol;
            MPI_Send(&offsetr,1,MPI_INT,i,1,MPI_COMM_WORLD);
            MPI_Send(&offsetc,1,MPI_INT,i,2,MPI_COMM_WORLD);
            MPI_Send(&rtemp,1,MPI_INT,i,3,MPI_COMM_WORLD);
            MPI_Send(&ctemp,1,MPI_INT,i,4,MPI_COMM_WORLD);
            MPI_Send(&snew[offsetr][offsetc],rtemp*ctemp,MPI_FLOAT,i,5,MPI_COMM_WORLD);
            offsetr += rtemp;
            offsetc += ctemp;
        }
        for(i = 1; i < p; i++)
        {
            MPI_Recv(&offsetr,1,MPI_INT,i,6,MPI_COMM_WORLD,&status);
            MPI_Recv(&offsetc,1,MPI_INT,i,7,MPI_COMM_WORLD,&status);
            MPI_Recv(&rtemp,1,MPI_INT,i,8,MPI_COMM_WORLD,&status);
            MPI_Recv(&ctemp,1,MPI_INT,i,9,MPI_COMM_WORLD,&status);
            MPI_Recv(&snew[offsetr][offsetc],rtemp*ctemp,MPI_FLOAT,i,10,MPI_COMM_WORLD,&status);
        }
    //do count round to calculate
    
        for(i=0;i<avgrow;i++){
            for(j=0;j<avgcol;j++){
                if((i==0) || (i==row-1) || (j==0) || (j==col-1)){
                        snew[i][j] = 255;
                }
                else
                        snew[i][j] = (s[i+1][j] + s[i-1][j] + s[i][j-1] + s[i][j+1])/4;
            }
        }
        
        for(i=0;i<row;i++)
        {
            for(j=0;j<col;j++)
            {
                s[i][j] = snew[i][j];
            }
        }
        printf("%d\n",k);
        for( i = 0; i <row; i++){
            for(j=0;j<col;j++){
                printf("%5.1f ",s[i][j]);
            }
                printf("\n");
        }
        printf("\n\n\n");
    }


    //print result
    
    endTime = MPI_Wtime();
    printf("Timings :%f Sec\n",endTime-startTime);
    MPI_Finalize();
    free(s);
    free(snew);

    return 0;
}
