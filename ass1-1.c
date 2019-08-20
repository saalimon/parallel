#include<stdio.h>
#include<mpi.h>
#include <stdlib.h>
int i,j,k;
float *initial_heat(int row,int col) {
    float *tmp = (float*) malloc((row*col)*sizeof(float));
    for(i=0; i< row;i++){
        for(j = 0; j < col; j++){
            // if( i==0 || j==0 || i == row-1 || j == col -1)
                tmp[i*col+j] = (i*col+j)*1.0;
            // else
            //     tmp[i*row+j] = 0;
        }
    }
    return tmp; 
}
int main(int argc,char** argv){
    float *s = NULL;
    int p,id;
    int avg;
    int extra;
    int blocksize;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    if (argc != 3) {
        fprintf(stderr, "Error Input please specify row and col in argv\n");
        exit(1);
    }
    int row = atoi(argv[1]);
    int col = atoi(argv[2]);
    if(id == 0){
       s = initial_heat(row,col);
        for( i = 0; i <row; i++){
            for(j=0;j<col;j++){
                printf("%5.1f ",s[i*col+j]);
            }
                printf("\n");
        }
        printf("\n");

    }
    int* displs = (int *)malloc(p*sizeof(int)); 
    int* scounts = (int *)malloc(p*sizeof(int)); 
    avg = row/p; //10/5 = 2
    extra = row%p; //10%2 = 0
    if(id==0)printf("%d %d\n",avg,extra);
    for (i=0; i<p; ++i) { 
        displs[i] = i==0?i*col:i*(avg+extra)*col;
        scounts[i] = i==0?(avg+extra)*col:i==p-1?avg*col:avg*col; 
    } 
    // if(id==0)for (i=0; i<p; ++i) printf("%d %d %d\n",i,displs[i],scounts[i]);
    float *sub_s = (float *)malloc(sizeof(float) * (avg+extra)*col);
    MPI_Scatterv(   &s[0], scounts, displs, MPI_FLOAT, 
                    sub_s, (avg+extra)*col, MPI_FLOAT, 0, MPI_COMM_WORLD); 
    // MPI_Scatter(s, 3, MPI_FLOAT, sub_s,3, MPI_FLOAT, 0, MPI_COMM_WORLD);
    for( i = 0; i < 2; i++)
    {
            if(id != 0){
            //printf("%d \n",id);
            int n = (id==p-1)?2:3;
            blocksize = avg*col;
            // for(i=0;i<n;i++){
                for(j=0;j<avg*col;j++){
                    sub_s[j] = n+i;
                    //printf("%5.1f ",sub_s[j]);
                }
            // }
                // printf("\n");
            }
            // else{
            //     int n = (id==0)?2:3;
            // //     //blocksize = avg*col;
            //     // for(i=0;i<n;i++){
            //         for(j=0; (avg+extra)*col;j++){
            //             sub_s[j] = n;
            //             //printf("%5.1f ",sub_s[j]);
            //         }
            // }
            MPI_Gatherv(&sub_s[0],blocksize,MPI_FLOAT,&s[id*(avg+extra)*col],scounts,displs,MPI_FLOAT,0,MPI_COMM_WORLD);
            //printf("fin\n");
            if(id==0){
                int n = (id==0)?2:3;
                blocksize = avg*col;
                // for(i=0;i<n;i++){
                    for(j=0;j<(avg+extra)*col;j++){
                        s[j] = n+i;
                        //printf("%5.1f ",s[j]);
                    }
            }
        
    }
    //MPI_Allgatherv(&s[0],(avg+extra)*col,MPI_FLOAT,&s[0],scounts,displs,MPI_FLOAT,MPI_COMM_WORLD);
    if(id == 0){
        for( i = 0; i <row; i++){
            for(j=0;j<col;j++){
                printf("%5.1f ",s[i*col+j]);
            }
                printf("\n");
        }
        printf("\n");
    }
        
        
    
    // if(id == 0){
    //      printf("%d ",id);
    //     for(i=0;i<2;i++){
    //         for(j=0;j<avg*col;j++){
    //             printf("%5.1f ",sub_s[i*avg*col+j]);
    //         }
    //     }
            
    //         printf("\n");
    // }
    MPI_Finalize();

    return 0;

}