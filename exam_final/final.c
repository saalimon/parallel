#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<omp.h> 
int main(int argc, char** argv)
{
    int i,j,k,l;
    int avg,extra;
    int t;
    int c;
    int id;
    int p;
    double* in;
    double* out;
    double num;
    double sum_kernal;
    double total_kernal;
    double *kernal = malloc(3*3*sizeof(double));;
    int row,col;
    int krow,kcol;
    FILE *fp;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    t = atoi(argv[4]);
    omp_set_num_threads(t);
    if(id == 0){
        total_kernal = 0;
        fp = fopen(argv[1], "r");
        if (fp) {
            i = 0;
            fscanf(fp,"%d %d\n",&row,&col);
            in = malloc(row*col*sizeof(double));
            while ((c = fscanf(fp,"%lf ",&num)) !=EOF){
                in[i++] = num;
            }
            fclose(fp);
        }

        fp = fopen(argv[2], "r");
        if(fp){
            i = 0;
            fscanf(fp,"%d %d\n",&krow,&kcol);
            kernal = malloc(krow*kcol*sizeof(double));
            while ((c = fscanf(fp,"%lf ",&num)) !=EOF){
                total_kernal += num;
                kernal[i++] = num;
            }
            fclose(fp);
        }
    }
    MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&col, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&total_kernal, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    printf("%d %d %lf\n",row,col,total_kernal);
    MPI_Bcast(kernal, 3*3, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    out = malloc(row*col*sizeof(double));
    printf("%d %d %d\n",id,row,col);
    int* displs = (int *)malloc(p*sizeof(int)); 
    int* scounts = (int *)malloc(p*sizeof(int)); 
    avg = row/p; //10/5 = 2
    avg = avg ;
    extra = row%p; //10%2 = 0
    int offset = 0;
    for (i=0; i<p; i++) { 
        displs[i] = offset;
        scounts[i] = i==0?(avg+extra)*col :(avg)*col; 
        if(i==0) offset += (avg+extra)*col;
        else offset += (avg)*col;
    } 
    double *sub_s = (double *)malloc(sizeof(double) * scounts[id]);
    MPI_Scatterv(   &in[0], scounts, displs, MPI_DOUBLE, 
                    sub_s, scounts[id], MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    if(id == 0){
        #pragma omp parallel for schedule(dynamic) num_threads(t) private(sum_kernal,i,j)
        for(i=0;i<avg+2;i++){
            for(j=0;j<col;j++){
                if((i==row-1)|| (i==0) || (j==0) || (j==col-1))
                    out[i*col+j] = in[i*col+j];
                else{
                    sum_kernal =    (kernal[8] * in[(i-1)*col+(j-1)]) + 
                                    (kernal[7] * in[(i)*col+(j-1)]) + 
                                    (kernal[6] * in[(i+1)*col+(j-1)]) + 
                                    (kernal[5] * in[(i-1)*col+(j)]) + 
                                    (kernal[4] * in[(i)*col+(j)]) +
                                    (kernal[3] * in[(i+1)*col+(j)]) +
                                    (kernal[2] * in[(i-1)*col+(j+1)]) +
                                    (kernal[1] * in[(i)*col+(j+1)]) +
                                    (kernal[0] * in[(i+1)*col+(j+1)]);
                    out[i*col+j] = sum_kernal/total_kernal;
                }
            }
        }
        for(i=1;i<p;i++){
            MPI_Recv(&out[displs[i]]+col, scounts[i], MPI_DOUBLE,   i, 1, MPI_COMM_WORLD, &status);
        }
        // MPI_Allgatherv(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL,
        //         out, scounts, displs, MPI_DOUBLE, MPI_COMM_WORLD);
        // MPI_Gatherv(&sub_s[0], avg*col, MPI_DOUBLE, out, scounts, displs, MPI_DOUBLE,0, MPI_COMM_WORLD);

        fp = fopen(argv[3], "w");
        if (fp) {
            fprintf(fp,"%d %d\n",row,col);
            for(i = 0;i<row*col;i++){
                    fprintf(fp,"%.0lf ",out[i]);
                if((i+1)%col==0)
                    fprintf(fp,"\n");
            }
        }
        fclose(fp);
    }
    else{
        printf("%d %d\n",avg,row);
        #pragma omp parallel for schedule(dynamic) num_threads(t) private(sum_kernal,i,j)
        for(i=0;i<avg+1;i++){
            for(j=0;j<col;j++){
                if(id == p-1){
                    if((i==row/p-1)|| (i==0) || (j==0) || (j==col-1)){
                        if((i==row/p-1)|| (j==0) || (j==col-1))
                            out[i*col+j] = sub_s[i*col+j];
                    }
                    else{
                        sum_kernal =    (kernal[8] * sub_s[(i-1)*col+(j-1)]) + 
                                        (kernal[7] * sub_s[(i)*col+(j-1)]) + 
                                        (kernal[6] * sub_s[(i+1)*col+(j-1)]) + 
                                        (kernal[5] * sub_s[(i-1)*col+(j)]) + 
                                        (kernal[4] * sub_s[(i)*col+(j)]) +
                                        (kernal[3] * sub_s[(i+1)*col+(j)]) +
                                        (kernal[2] * sub_s[(i-1)*col+(j+1)]) +
                                        (kernal[1] * sub_s[(i)*col+(j+1)]) +
                                        (kernal[0] * sub_s[(i+1)*col+(j+1)]);
                        out[i*col+j] = sum_kernal/total_kernal;
                    }
                }
                else{
                    if( (i==0) || (j==0) || (j==col-1)){
                        if((j==0) || (j==col-1))
                            out[i*col+j] = sub_s[i*col+j];
                    }
                    else{
                        sum_kernal =    (kernal[8] * sub_s[(i-1)*col+(j-1)]) + 
                                        (kernal[7] * sub_s[(i)*col+(j-1)]) + 
                                        (kernal[6] * sub_s[(i+1)*col+(j-1)]) + 
                                        (kernal[5] * sub_s[(i-1)*col+(j)]) + 
                                        (kernal[4] * sub_s[(i)*col+(j)]) +
                                        (kernal[3] * sub_s[(i+1)*col+(j)]) +
                                        (kernal[2] * sub_s[(i-1)*col+(j+1)]) +
                                        (kernal[1] * sub_s[(i)*col+(j+1)]) +
                                        (kernal[0] * sub_s[(i+1)*col+(j+1)]);
                        // printf("%.0lf\n",sum_kernal/total_kernal);
                        out[i*col+j] = sum_kernal/total_kernal;
                    }
                }
                
            }
        }
        MPI_Send(&out[col], scounts[id], MPI_DOUBLE,   0, 1, MPI_COMM_WORLD); 
    }

    MPI_Finalize();
    return 0;
}    