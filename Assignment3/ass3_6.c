#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<omp.h> 
#include<math.h>
int main(int argc,char** argv)
{
    int i,j,k,t;
    int id,p;
    FILE *fp;
    MPI_Status status;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id); 
    MPI_Request request,request1,request2,request3,request4,request5,request6;
    // MPI_Status status;
    int N,K;
    double d;
    double *m;
    double *q;
    double *x;
    double *y;
    double *vx;
    double *vy;
    // double *Fx;
    // double *Fy;
    double *Ax;
    double *Ay;
    double *dax;
    double *day;
    double *dx;
    double *dy;
    double *dvx;
    double *dvy;
    double startTime;
    double endTime;
    int disps[p];
    int recvcounts[p];
    t = atoi(argv[3]);
    omp_set_num_threads(t);
    if(id ==0){
        fp = fopen(argv[1], "r");
        if(fp){
            fscanf(fp,"%d %d\n%lf\n",&N,&K,&d);   
        }
    }
    MPI_Ibcast(&N,1,MPI_INT,0,MPI_COMM_WORLD,&request1);
    MPI_Ibcast(&K,1,MPI_INT,0,MPI_COMM_WORLD,&request2);
    MPI_Ibcast(&d,1,MPI_DOUBLE,0,MPI_COMM_WORLD,&request3);
    MPI_Wait(&request1, MPI_STATUS_IGNORE);  
    m   = (double*) malloc(N*sizeof(double));
    q   = (double*) malloc(N*sizeof(double));
    x   = (double*) calloc(N,sizeof(double));
    y   = (double*) calloc(N,sizeof(double));
    vx  = (double*) malloc(N*sizeof(double));
    vy  = (double*) malloc(N*sizeof(double));
    if(id == 0){
        for( i = 0; i < N; i++)
            {
                fscanf(fp,"%lf %lf %lf %lf %lf %lf\n",&m[i],&q[i],&x[i],&y[i],&vx[i],&vy[i]);
            }
            fclose(fp);
            
    }
    Ax = (double*) calloc(N,sizeof(double));
    Ay = (double*) calloc(N,sizeof(double));
    dax = (double*) calloc(N,sizeof(double));
    day = (double*) calloc(N,sizeof(double));
    dx = (double*) calloc(N,sizeof(double));
    dy = (double*) calloc(N,sizeof(double));
    dvx = (double*) calloc(N,sizeof(double));
    dvy = (double*) calloc(N,sizeof(double));
    double ftemp;
    double fx;
    double fy;
    int data_chunk = N/p;
    int mod_part = N%p;
    if(id == 0) printf("chunk = %d mod = %d\n",data_chunk,N%p);
    int start = id==0?data_chunk*id:(data_chunk*id)+mod_part;
    int end = id==0?data_chunk*(id+1)+mod_part:data_chunk*(id+1)+mod_part;
    printf("%d start = %d end = %d\n",id,start,end);
    for(int l = 0;l<p;l++){
        int pos = l == 0 ? data_chunk*l:(data_chunk*l)+mod_part;
        int size = l == 0 ? data_chunk+mod_part:data_chunk;
        disps[l] = pos;
        recvcounts[l] = size;
    }

    MPI_Wait(&request2, MPI_STATUS_IGNORE); 
    MPI_Wait(&request3, MPI_STATUS_IGNORE);
    MPI_Bcast(m,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(q,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(x,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(y,N,MPI_DOUBLE,0,MPI_COMM_WORLD);    
    MPI_Bcast(vx,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(vy,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    if(id == 0){
        fp = fopen(argv[2], "w");
        startTime = MPI_Wtime();
    }
    // for(int l = 0;l<p;l++){
    //     if(id == 0) printf("%d\n",disps[l]);
    // }
    for(i=0;i<K+1;i++){
        for(j=start;j<end;j++){
                if(abs(x[j])<1000000&&abs(y[j])<1000000)
                {
                    vx[j] = vx[j]+dax[j];
                    vy[j] = vy[j]+day[j];
                    x[j] = x[j] + dvx[j];
                    y[j] = y[j] + dvy[j];
                }
        }
        // MPI_Allgatherv();
        int size = id == 0 ? data_chunk+mod_part:data_chunk;
        MPI_Allgatherv(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL,
                x, recvcounts, disps, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Allgatherv(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL,
                y, recvcounts, disps, MPI_DOUBLE, MPI_COMM_WORLD);
        if(id == 0){
            endTime = MPI_Wtime();
            printf("%d Timings :%f Sec\n",i,endTime-startTime);
            for (j=0;j<N;j++)
            {
                if(abs(x[j])<1000000&&abs(y[j])<1000000)
                    fprintf(fp,"%.6lf %.6lf\n",x[j],y[j]);
                else
                    fprintf(fp,"out\n");
            }
            fprintf(fp,"---\n");
        }
        #pragma omp parallel for schedule(dynamic) num_threads(t) private( ftemp, fx, fy,k)
            for(j=start;j<end;j++){
                fx = 0;
                fy = 0;
                if(abs(x[j])<1000000&&abs(y[j])<1000000)
                {
                    for(k=0;k<N;k++){
                        if(k!=j){
                            if(abs(x[k])<1000000&&abs(y[k])<1000000)
                            {
                                ftemp = (q[j]*q[k]) /(pow((x[j]-x[k]),2.0)+ pow((y[j]-y[k]),2.0));
                                fx += ftemp*((x[j]-x[k]))/sqrt(pow(x[j]-x[k],2.0)+ pow((y[j]-y[k]),2.0));
                                fy += ftemp*((y[j]-y[k]))/sqrt(pow(x[j]-x[k],2.0)+ pow((y[j]-y[k]),2.0));
                            }
                        }
                    }
                }
                Ax[j] =  fx/m[j]; // find a in time = t
                Ay[j] =  fy/m[j]; // find a in time = t
                dax[j] = d * Ax[j]; // find a in time = t + d
                day[j] = d * Ay[j]; // find a in time = t + d
                dvx[j] = d * vx[j]; // find vx in time = t + d
                dvy[j] = d * vy[j]; // find vy in time = t + d
                
            }
    }
    
    if(id == 0){
        fclose(fp);
        endTime = MPI_Wtime();
        printf("%d Timings :%f Sec\n",i,endTime-startTime);
    }
    MPI_Finalize();
    return 0;
}
