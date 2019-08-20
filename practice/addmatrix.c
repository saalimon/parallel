#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char** argv)
{
    int i;
    int id;
    int p;
    int* x;
    int* y;
    int* z;
    int size;
    int mod;
    int n;

    FILE *fp;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    if (id == 0){
        fp = fopen(argv[1], "r");
        if (fp) {
            fscanf(fp,"%d\n",&n);
            x = malloc(n*sizeof(int));
            for(i = 0;i<n;i++){
                fscanf(fp,"%d\n",&x[i]);
            }
            fclose(fp);
        }
        fp = fopen(argv[2], "r");
        if (fp) {
            fscanf(fp,"%d\n",&n);
            y = malloc(n*sizeof(int));
            for(i = 0;i<n;i++){
                fscanf(fp,"%d\n",&y[i]);
            }
            fclose(fp);
        }
        z = malloc(n*sizeof(int));

    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(id == 0){
        size = n/p;
        mod = n%p;
        int start = 0;
        int stop = start + size +mod;
        for(i = start;i<stop;i++){
            z[i] = x[i]+y[i];
            // printf("%d>> %d %d\n",id,x[i],y[i]);
        }
        for(i=1;i<p;i++){
            MPI_Send(&x[size*i+mod], size, MPI_INT,   i, 1, MPI_COMM_WORLD); 
            MPI_Send(&y[size*i+mod], size, MPI_INT,   i, 2, MPI_COMM_WORLD);
            MPI_Recv(&z[size*i+mod], size, MPI_INT,   i, 3, MPI_COMM_WORLD, &status);
        } 
        for(i = 0;i<n;i++){
            printf("%d\n",z[i]);
        }

    }
    else{
        size = n/p;
        mod = n%p;  
        x = malloc(n*sizeof(int));
        y = malloc(n*sizeof(int));
        z = malloc(n*sizeof(int));
        MPI_Recv(&x[size*id+mod], size, MPI_INT,   0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&y[size*id+mod], size, MPI_INT,   0, 2, MPI_COMM_WORLD, &status);
        int start = size*id+mod;
        int stop = start + size;
        for(i = start;i<stop;i++){
            z[i] = x[i]+y[i];
        }
        MPI_Send(&z[size*id+mod], size, MPI_INT,   0, 3, MPI_COMM_WORLD); 
    }

    MPI_Finalize();
    return 0;
}