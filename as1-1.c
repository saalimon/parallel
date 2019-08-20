#include<stdio.h>
#include<mpi.h>
#include <stdlib.h>
#include <string.h>

int i,j,k;
float *initial_heat(int row,int col) {
    float *tmp = (float*) malloc((row*col)*sizeof(float));
    for(i=0; i< row;i++){
        for(j = 0; j < col; j++){
            if( i==0 || j==0 || i == row-1 || j == col -1){
                                // tmp[i*col+j] = (i*col+j)*255.0;
                tmp[i*col+j] = 255.0;
            }
            else
                tmp[i*col+j] = 0.0;
        }
    }
    return tmp; 
}
int main(int argc,char** argv){
    float *s = NULL;
    int p,id;
    int avg;
    int extra;
    int row,col;
    int count;
    double startTime;
    double endTime;
    int flag;
    FILE *fp;
    MPI_Request request;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    if (argc != 4) {
        fprintf(stderr, "Error Input please specific file name of input\n");
        exit(1);
    }
    if(id == 0){
        if(argc == 4){
            fp = fopen(argv[1], "r");
            if (fp) {
                fscanf(fp,"%d %d\n",&row,&col);          
                fclose(fp);
            }
            count =atoi(argv[3]);
        }
        
    }
    MPI_Bcast(&row,    1,                      MPI_INT,    0,  MPI_COMM_WORLD);
    MPI_Bcast(&col,    1,                      MPI_INT,    0,  MPI_COMM_WORLD);
    MPI_Bcast(&count,    1,                      MPI_INT,    0,  MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if(id == 0){
       s = initial_heat(row,col);
    }
    startTime = MPI_Wtime();
    MPI_Barrier(MPI_COMM_WORLD);
    int* displs = (int *)malloc(p*sizeof(int)); 
    int* scounts = (int *)malloc(p*sizeof(int)); 
    avg = row/p; //10/5 = 2
    extra = row%p; //10%2 = 0
    int offset = 0;
    for (i=0; i<p; i++) { 
        displs[i] = offset;
        scounts[i] = i==0?(avg+extra)*col:(avg)*col; 
        if(i==0) offset += (avg+extra)*col;
        else offset += (avg)*col;
    } 
    float *sub_s = (float *)malloc(sizeof(float) * (avg+extra)*col);
    MPI_Scatterv(   &s[0], scounts, displs, MPI_FLOAT, 
                    sub_s, (avg+extra)*col, MPI_FLOAT, 0, MPI_COMM_WORLD); 
for(int round=0;round<count;round++)
    {
    if(id==0){
        float *snew = (float*) malloc((row*col)*sizeof(float));
        flag = 0;
        for (i=1; i<p; ++i) {
            if(i == p-1){
                //send head only
                MPI_Isend(&s[displs[i]-col],col,MPI_INT,i,1,MPI_COMM_WORLD,&request);
            }else{
                //send head and tail
                MPI_Isend(&s[displs[i]-col],col,MPI_INT,i,1,MPI_COMM_WORLD,&request);
                MPI_Isend(&s[displs[i+1]],col,MPI_INT,i,2,MPI_COMM_WORLD,&request);
            }
        }
        for(i=0;i<avg+extra;i++){
            for(j=0;j<col;j++){
                if((i==row-1)|| (i==0) || (j==0) || (j==col-1))
                    snew[i*col+j] = 255.0;
                else{
                    snew[i*col+j] = (s[(i+1)*col+j]+s[(i-1)*col+j]+s[i*col+(j-1)]+s[i*col+(j+1)])/4.0;
                }
            }
        }
        for(i=0;i<avg+extra;i++){
            for(j=0;j<col;j++){
                sub_s[i*col+j] = snew[i*col+j];
            }
        }
        free(snew);
        // MPI_Gatherv(&sub_s[0], (avg+extra)*col, MPI_FLOAT, s, scounts, displs, MPI_FLOAT,0, MPI_COMM_WORLD);
    }
    if(id!=0){
        float *snew = (float*) malloc((row*col)*sizeof(float));
        float *head = (float*) malloc(col*sizeof(float));
        float *tail = (float*) malloc(col*sizeof(float));
        if(id == p-1){ //final part
            MPI_Recv(&head[0],col,MPI_INT,0,1,MPI_COMM_WORLD,&status);
            for(i=0;i<avg;i++){
                for(j=0;j<col;j++){
                    if(i==avg-1||j==0||j==col-1){
                        snew[i*col+j] = 255.0;
                    }
                    else{
                        if(i==0){
                            snew[i*col+j] = (head[j]+sub_s[(i+1)*col+j]+sub_s[i*col+(j-1)]+sub_s[i*col+(j+1)])/4.0;
                        }
                        else{
                            snew[i*col+j] = (sub_s[(i-1)*col+j]+sub_s[(i+1)*col+j]+sub_s[i*col+(j-1)]+sub_s[i*col+(j+1)])/4.0;
                        }
                    }
                }
            }
            for(i=0;i<avg;i++){
                for(j=0;j<col;j++){
                    sub_s[i*col+j] = snew[i*col+j];
                }
            }
        }
        else{ //other part
            MPI_Recv(&head[0],col,MPI_INT,0,1,MPI_COMM_WORLD,&status);
            MPI_Recv(&tail[0],col,MPI_INT,0,2,MPI_COMM_WORLD,&status);
            for(i=0;i<avg;i++){
                for(j=0;j<col;j++){
                    if(i==row-1||j==0||j==col-1){
                        snew[i*col+j] = 255.0;
                    }
                    else{
                        if(i==0){
                            snew[i*col+j] = (head[j]+sub_s[(i+1)*col+j]+sub_s[i*col+(j-1)]+sub_s[i*col+(j+1)])/4.0;
                        }
                        else if(i<avg-1){
                            snew[i*col+j] = (sub_s[(i-1)*col+j]+sub_s[(i+1)*col+j]+sub_s[i*col+(j-1)]+sub_s[i*col+(j+1)])/4.0;
                        }
                        else{
                            snew[i*col+j] = (sub_s[(i-1)*col+j]+tail[j]+sub_s[i*col+(j-1)]+sub_s[i*col+(j+1)])/4.0;
                        }
                    }
                }
            }
            for(i=0;i<avg;i++){
                for(j=0;j<col;j++){
                    sub_s[i*col+j] = snew[i*col+j];
                }
            }
        }
        free(snew);
        free(head);
        free(tail);
        // MPI_Gatherv(&sub_s[0], avg*col, MPI_FLOAT, s, scounts, displs, MPI_FLOAT,0, MPI_COMM_WORLD);
    } 
    MPI_Gatherv(&sub_s[0], scounts[id], MPI_FLOAT, s, scounts, displs, MPI_FLOAT,0, MPI_COMM_WORLD);
}
free(sub_s);  
endTime = MPI_Wtime();

if(id == 0){
    printf("Timing = %lf sec.\n",endTime-startTime);
    fp = fopen(argv[2], "w");
    if (fp) {
        fprintf(fp,"%d %d\n",row,col); 
        for(i = 0;i<row;i++){
            for(j= 0;j<col;j++){
                fprintf(fp,"%.0f ",s[i*col+j]);
            }
            fprintf(fp,"\n");
        }         
        fclose(fp);
    }
}
free(s);
MPI_Finalize();
return 0;

}