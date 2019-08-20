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
    float *s;
    float *snew;
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
        s = (float*) malloc((row*col)*sizeof(float));
        for(i=0; i< row;i++){
            for(j = 0; j < col; j++){
                if( i==0 || j==0 || i == row-1 || j == col -1)
                    s[i*row+j] = 255;
                else
                    s[i*row+j] = 0;
            }
        }
        snew = (float*) malloc((row*col)*sizeof(float));
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
    //do count round to calculate
    
        for(i=0;i<avgrow;i++){
            for(j=0;j<avgcol;j++){
                if((i==0) || (i==row-1) || (j==0) || (j==col-1)){
                        snew[i*avgrow+j] = 255;
                }
                else
                        snew[i*avgrow+j] = (s[(i+1)*avgrow+j] + s[(i-1)*avgrow+j] + s[(i)*avgrow+j-1] + s[(i)*avgrow+j+1])/4;
            }
        }
        
        for(i=0;i<row;i++)
        {
            for(j=0;j<col;j++)
            {
                s[i*row+j] = snew[i*row+j];
            }
        }
        
        printf("%d\n",k);
        for( i = 0; i <row; i++){
            for(j=0;j<col;j++){
                printf("%5.1f ",s[i*row+j]);
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
