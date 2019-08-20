#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
int main(int argc,char** argv)
{
    int i;
    int c;
    int id;
    int p;
    int total;
    int nrow,ncol;
    int slave_loc;
    int start_loc;
    double num;
    double *matA;

    double *matB;
    FILE *fp;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    if(id == 0){    //read file of file 1
        i = 0;
        fp = fopen(argv[1], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&nrow,&ncol);
            matA = malloc(nrow * ncol *sizeof(double));
            while ((c = fscanf(fp,"%lf ",&num)) !=EOF){
                matA[i++] = num;
                printf("%lf\n",num);
            }
                //MPI_Send(c,sizeof(double),MPI_FLOAT,1,1,MPI_COMM_WORLD);
            fclose(fp);
        }
        fp = fopen(argv[3], "w");
        if (fp) {
            fprintf(fp,"%d %d\n",nrow,ncol);
            for(i=0;i<nrow;i++)
            {
                for(j=0;j<ncol;j++)
                {
                    fprintf(fp,"%lf ",matAB[i]);
                }
                fprintf("\n");
            }
        }
                //MPI_Send(c,sizeof(double),MPI_FLOAT,1,1,MPI_COMM_WORLD);
            fclose(fp);
        }

    }


   MPI_Finalize();


    return 0;
}
