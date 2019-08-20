#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    int i;
    int id;
    int p;
    char *buffer;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    if (id == 0)
    {
        for (i = 1 ; i< p ; i++)
        {
            MPI_Recv(buffer,50,MPI_UNSIGNED_CHAR,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            printf("%s",buffer);
        }
    }
    else
    {
        sprintf (buffer, "Hello world rank 0 I am rank %d\n",id);
        MPI_Send(buffer,50,MPI_UNSIGNED_CHAR,0,0,MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
