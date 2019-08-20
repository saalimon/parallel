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
                                // tmp[i*col+j] = (i*col+j)*1.0;
                tmp[i*col+j] = 1.0;
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
    FILE *fp;
    MPI_Request request[6];
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    if (argc < 2||argc > 3) {
        fprintf(stderr, "Error Input please specific file name of input\n");
        exit(1);
    }
    if(id == 0){
        if(argc == 2){
            fp = fopen(argv[1], "r");
            if (fp) {
                fscanf(fp,"%d %d\n",&row,&col);          
                fclose(fp);
            }
        }
        else{
            row = atoi(argv[1]);
            col = atoi(argv[2]);
        }
        
    }
    MPI_Bcast(&row,    1,                      MPI_INT,    0,  MPI_COMM_WORLD);
    MPI_Bcast(&col,    1,                      MPI_INT,    0,  MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
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
    int offset = 0;
    // if(id==0)printf("%d %d\n",avg,extra);
    for (i=0; i<p; i++) { 
        displs[i] = offset;
        // displs[i] = i==0?i*col:i*(avg+extra)*col;
        scounts[i] = i==0?(avg+extra)*col:(avg)*col; 
        if(i==1) offset += (avg+extra)*col;
        else offset += (avg)*col;
    } 
    // if(id==0)for (i=0; i<p; ++i) printf("%d %d %d\n",i,displs[i],scounts[i]);
    float *sub_s = (float *)malloc(sizeof(float) * (avg+extra)*col);
    MPI_Scatterv(   &s[0], scounts, displs, MPI_FLOAT, 
                    sub_s, (avg+extra)*col, MPI_FLOAT, 0, MPI_COMM_WORLD); 
    // MPI_Scatter(s, 3, MPI_FLOAT, sub_s,3, MPI_FLOAT, 0, MPI_COMM_WORLD);
    if(id==0){
        float *snew = (float*) malloc((row*col)*sizeof(float));
        for(i=0;i<avg+extra;i++){
            for(j=0;j<col;j++){
                if((i==0) || (j==0) || (j==col-1))
                    snew[i*col+j] = 1.0;
                else{
                    int up = (i+1)*col+j;       //up
                    int down = (i-1)*col+j;     //down
                    int left = i*col+(j-1);     //left
                    int right = i*col+(j+1);    //right
                    snew[i*col+j] = (s[up]+s[down]+s[left]+s[right])/4.0;
                }
            }
        }
        for(i=0;i<avg+extra;i++){
            for(j=0;j<col;j++){
                printf("%5.1f ",snew[i*col+j]);
            }
            printf("\n");
        }

        for (i=1; i<p; ++i) {
            if(i == p-1){
                //send head only
                // printf("id: %d head:%d %d\n",i,displs[i]-col,displs[i]);
                int head = displs[i]-col;
                MPI_Send(&s[head],col,MPI_INT,i,1,MPI_COMM_WORLD);
            }else{
                //send head and tail
                // printf("%d %d %d %d\n",i,displs[i]-col,displs[i],displs[i]+col);
                int head = displs[i]-col;
                int tail = displs[i+1];
                MPI_Send(&s[head],col,MPI_INT,i,1,MPI_COMM_WORLD);
                MPI_Send(&s[tail],col,MPI_INT,i,2,MPI_COMM_WORLD);
            }
        }
        //MPI_Gatherv(&snew[0],(avg+extra)*col,MPI_FLOAT,&s[0],scounts,displs,MPI_FLOAT,0,MPI_COMM_WORLD);
        // MPI_Allgather(&sub_avg, 200, MPI_FLOAT, sub_avgs, 200, MPI_FLOAT, MPI_COMM_WORLD);
    }
    if(id!=0){
        float *snew = (float*) malloc((row*col)*sizeof(float));
        float *head = (float*) malloc(col*sizeof(float));
        float *tail = (float*) malloc(col*sizeof(float));
        if(id == p-1){ //final part
            MPI_Recv(&head[0],col,MPI_INT,0,1,MPI_COMM_WORLD,&status);
            //head calculator
            for(i = 0;i<1;i++){
                for(j=0;j<col;j++){
                    if((j==0) || (j==col-1))
                        snew[i*col+j] = 1.0;
                    else{
                        int up = j;                 //up
                        int down = (i-1)*col+j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (head[up]+sub_s[down]+sub_s[left]+sub_s[right])/4.0;
                        // printf("%5.1f ",snew[i*col+j]);
                    }
                }
            }
            //body calculator
            for(i=1;i<avg;i++){
                for(j=0;j<col;j++){
                    if((i==avg-1) || (j==0) || (j==col-1))
                        snew[i*col+j] = 1.0;
                    else{
                        int up = (i+1)*col+j;       //up
                        int down = (i-1)*col+j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (sub_s[up]+sub_s[down]+sub_s[left]+sub_s[right])/4.0;
                        // printf("%5.1f %5.1f %5.1f %5.1f ",sub_s[up],sub_s[down],sub_s[left],sub_s[right]);
                        // printf("%5.1f ",snew[i*col+j]);
                    }
                }
            }
            for(i=0;i<avg;i++){
                for(j=0;j<col;j++){
                    printf("%5.1f ",snew[i*col+j]);
                }
                printf("\n");
            }
        }
        else{ //other part
            MPI_Recv(&head[0],col,MPI_INT,0,1,MPI_COMM_WORLD,&status);
            MPI_Recv(&tail[0],col,MPI_INT,0,2,MPI_COMM_WORLD,&status);
            //head calculator
            for(i = 0;i<1;i++){
                for(j=0;j<col;j++){
                    if((j==0) || (j==col-1))
                        snew[i*col+j] = 1.0;
                    else{
                        int up = j;                 //up
                        int down = (i-1)*col+j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (head[up]+sub_s[down]+sub_s[left]+sub_s[right])/4.0;
                        // printf("%5.1f ",snew[i*col+j]);
                    }
                }
            }
            //body calculator
            for(i=1;i<avg;i++){
                for(j=0;j<col;j++){
                    if((i==avg-1) || (j==0) || (j==col-1))
                        snew[i*col+j] = 1.0;
                    else{
                        int up = (i+1)*col+j;       //up
                        int down = (i-1)*col+j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (sub_s[up]+sub_s[down]+sub_s[left]+sub_s[right])/4.0;
                        // printf("%5.1f %5.1f %5.1f %5.1f ",sub_s[up],sub_s[down],sub_s[left],sub_s[right]);
                        // printf("%5.1f ",snew[i*col+j]);
                    }
                }
            }
            //tail calculator
            for(i = avg-1;i<avg;i++){
                for(j=0;j<col;j++){
                    if((j==0) || (j==col-1))
                        snew[i*col+j] = 1.0;
                    else{
                        int up = (i+1)*col+j;                 //up
                        int down = j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (sub_s[up]+tail[down]+sub_s[left]+sub_s[right])/4.0;
                        // printf("%5.1f ",snew[i*col+j]);
                    }
                }
            }
            for(i=0;i<avg;i++){
                for(j=0;j<col;j++){
                    printf("%5.1f ",snew[i*col+j]);
                }
                printf("\n");
            }
        }
        MPI_Gatherv(&snew[0],avg*col,MPI_FLOAT,&s[0],scounts,displs,MPI_FLOAT,0,MPI_COMM_WORLD);
    }
    printf("\n");
    //int n = id==0?(avg+extra)*col:avg*col;
    for(i = 0; i < row; i++)
    {
        for(j=0;j<col;j++){
            printf("%5.1f ",s[i*col+j]);
        }            
        printf("\n");
    }
    
    MPI_Finalize();

    return 0;

}





//MPI_Gatherv(&snew[0],(avg+extra)*col,MPI_FLOAT,&s[id*(avg+extra)*col],scounts,displs,MPI_FLOAT,0,MPI_COMM_WORLD);