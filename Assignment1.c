#include<stdio.h>
#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include <stddef.h>
int main(int argc,char** argv)
{
    int id;
    int p;
    int i,j;
    int size = 12;
    float **s;
    float **snew;
    float diffnorm = 0;
    double startTime;
    double endTime;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    //initial
    startTime = MPI_Wtime();
    for(i=0;i<size;i++)
    {
        for(j = 0; j < size; i++)
        {
            if((i == 0) || (i == size-1)|| (j == 0)||(j == size-1))
                s[i][j]= 255;
            else
            {
                s[i][j]=0;
            }
            printf("%f ",s[i][j]);
        }
        printf("\n");
    }

    //end
    
    while(diffnorm < 0.01)
    {
        if(id==0)
        {
            for(i=0;i<size;i++)
            {
                for(j=0;j<size;j++)
                {
                    snew[i][j] = (s[i+1][j]+s[i-1][j]+s[i][j-1]+s[i][j+1])/4;
                }
            }
            // for(i = 0; i < size; i++)
            // {
            //     for(j = 0; j < size; j++)
            //     {
            //         diffnorm += ( snew[i][j] - s[i][j] )*( snew[i][j] - s[i][j] );
            //     }
            // }
            // diffnorm = sqrt(diffnorm);
            // MPI_Allreduce();
            for(i=0;i<size;i++)
            {
                for(j=0;j<size;j++)
                {
                    s[i][j]=snew[i][j];
                }
            }
        }
        if(id!=0)
        {
            
        }
    }
    endTime = MPI_Wtime();
    printf("Timings :%f Sec\n",endTime-startTime);
    
    
    MPI_Finalize();
    return 0;
}