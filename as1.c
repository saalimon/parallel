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
    FILE *fp;
    MPI_Request request[6];
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
for(int round=0;round<count;round++)
    {
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
    if(id == 0){
        for(i=1;i<p;i++){
            MPI_Send(&s[displs[i]], scounts[i], MPI_FLOAT, i, 0, MPI_COMM_WORLD);
        }
    }
    if(id==0){
        float *sub_s = (float *)malloc(sizeof(float) * scounts[id]);
        float *snew = (float*) malloc((row*col)*sizeof(float));
        for(i=0;i<avg+extra;i++){
            for(j=0;j<col;j++){
                if((i==row-1)|| (i==0) || (j==0) || (j==col-1))
                    snew[i*col+j] = 255.0;
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
                s[i*col+j] = snew[i*col+j];
            }
        }

        for (i=1; i<p; ++i) {
            if(i == p-1){
                //send head only
                int head = displs[i]-col;
                MPI_Send(&s[head],col,MPI_INT,i,1,MPI_COMM_WORLD);
            }else{
                //send head and tail
                int head = displs[i]-col;
                int tail = displs[i+1];
                MPI_Send(&s[head],col,MPI_INT,i,1,MPI_COMM_WORLD);
                MPI_Send(&s[tail],col,MPI_INT,i,2,MPI_COMM_WORLD);
            }
        }
        for(i=1;i<p;i++){
            MPI_Recv(&s[displs[i]],scounts[i],MPI_FLOAT,i,3,MPI_COMM_WORLD,&status);
        }
        free(snew);
        // MPI_Gatherv(&sub_s[0], (avg+extra)*col, MPI_FLOAT, s, scounts, displs, MPI_FLOAT,0, MPI_COMM_WORLD);
    }
    if(id!=0){
        float *snew = (float*) malloc((row*col)*sizeof(float));
        float *head = (float*) malloc(col*sizeof(float));
        float *tail = (float*) malloc(col*sizeof(float));
        float *sub_s = (float *)malloc(sizeof(float) * scounts[id]);
        if(id == p-1){ //final part        
            MPI_Recv(&sub_s[0],scounts[id],MPI_FLOAT,0,0,MPI_COMM_WORLD,&status);
            MPI_Recv(&head[0],col,MPI_INT,0,1,MPI_COMM_WORLD,&status);
            //head calculator
            for(i = 0;i<1;i++){
                for(j=0;j<col;j++){
                    if((i==avg-1)|| (j==0) || (j==col-1))
                        snew[i*col+j] = 255.0;
                    else{
                        int up = j;                 //up
                        int down = (i+1)*col+j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (head[up]+sub_s[down]+sub_s[left]+sub_s[right])/4.0;
                    }
                }
            }
            //body calculator
            for(i=1;i<avg;i++){
                for(j=0;j<col;j++){
                    if((i==avg-1)|| (i==0) || (j==0) || (j==col-1))
                        snew[i*col+j] = 255.0;
                    else{
                        int up = (i-1)*col+j;       //up
                        int down = (i+1)*col+j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (sub_s[up]+sub_s[down]+sub_s[left]+sub_s[right])/4.0;
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
            MPI_Recv(&sub_s[0],scounts[id],MPI_FLOAT,0,0,MPI_COMM_WORLD,&status);
            MPI_Recv(&head[0],col,MPI_INT,0,1,MPI_COMM_WORLD,&status);
            MPI_Recv(&tail[0],col,MPI_INT,0,2,MPI_COMM_WORLD,&status);
            //head calculator
            for(i = 0;i<1;i++){
                for(j=0;j<col;j++){
                    if((j==0) || (j==col-1))
                        snew[i*col+j] = 255.0;
                    else{
                        int up = j;                 //up
                        int down = (i+1)*col+j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (head[up]+sub_s[down]+sub_s[left]+sub_s[right])/4.0;
                    }
                }
            }
            //body calculator
            for(i=1;i<avg;i++){
                for(j=0;j<col;j++){
                    if((i==0) || (j==0) || (j==col-1))
                        snew[i*col+j] = 255.0;
                    else{
                        int up = (i-1)*col+j;       //up
                        int down = (i+1)*col+j;     //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (sub_s[up]+sub_s[down]+sub_s[left]+sub_s[right])/4.0;
                    }
                }
            }
            //tail calculator
            for(i = avg-1 ;i<avg;i++){
                for(j=0;j<col;j++){
                    if((j==0) || (j==col-1))
                        snew[i*col+j] = 255.0;
                    else{
                        int up = (i-1)*col+j;       //up
                        int down = j;               //down
                        int left = i*col+(j-1);     //left
                        int right = i*col+(j+1);    //right
                        snew[i*col+j] = (sub_s[up]+tail[j]+sub_s[left]+sub_s[right])/4.0;
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
        MPI_Send(&sub_s[0], scounts[id], MPI_FLOAT, 0, 3, MPI_COMM_WORLD);
        // MPI_Gatherv(&sub_s[0], avg*col, MPI_FLOAT, s, scounts, displs, MPI_FLOAT,0, MPI_COMM_WORLD);
        free(sub_s);  
    } 
//     for(i=0;i<row;i++){
//     for(j=0;j<col;j++){
//         fprintf(fp,"%.0f ",s[i*col+j]);
//     }
//     fprintf(fp,"\n");
// }
// fclose(fp);
}
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