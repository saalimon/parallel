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
    MPI_Request request1, request2, request3, request4;
    // MPI_Status status;
    int N,K;
    double d;
    double *m;
    double *q;
    double *x;
    double *y;
    double *vx;
    double *vy;
    double *Fx;
    double *Fy;
    double *Ax;
    double *Ay;
    double *dax;
    double *day;
    double *dx;
    double *dy;
    double *dvx;
    double *dvy;
    t = atoi(argv[3]);
    omp_set_num_threads(t);
    if(id ==0){
        fp = fopen(argv[1], "r");
        if(fp){
            fscanf(fp,"%d %d\n%lf\n",&N,&K,&d);   
        }
    }
    MPI_Bcast(&N,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&K,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&d,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
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
    MPI_Bcast(m,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(q,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(x,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(y,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(vx,N,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(vy,N,MPI_DOUBLE,0,MPI_COMM_WORLD);

    Fx = (double*) calloc(N,sizeof(double));
    Fy = (double*) calloc(N,sizeof(double));
    Ax = (double*) calloc(N,sizeof(double));
    Ay = (double*) calloc(N,sizeof(double));
    dax = (double*) calloc(N,sizeof(double));
    day = (double*) calloc(N,sizeof(double));
    dx = (double*) calloc(N,sizeof(double));
    dy = (double*) calloc(N,sizeof(double));
    dvx = (double*) calloc(N,sizeof(double));
    dvy = (double*) calloc(N,sizeof(double));
    MPI_Barrier(MPI_COMM_WORLD);
    double ftemp;
    double fx;
    double fy;
    int data_chunk = N/p;
    int mod_part = N%p;
    int start = id==0?data_chunk*id:(data_chunk*id)+mod_part;
    int end = id==0?data_chunk*(id+1)+mod_part:data_chunk*(id+1);
    if(id == 0){
        fp = fopen(argv[2], "w");
    }

    for(i=0;i<K+1;i++){
        #pragma omp for schedule(dynamic) num_threads(t)
        for(j=start;j<end;j++){
            #pragma omp single
            {
                vx[j] = vx[j]+dax[j];
                vy[j] = vy[j]+day[j];
                x[j] = x[j] + dvx[j];
                y[j] = y[j] + dvy[j];
            }
        }
        for(int l = 0;l<p;l++){
            if(l != id){
                int size = id == 0 ? data_chunk+mod_part:data_chunk;
                MPI_Send(&x[start], size, MPI_DOUBLE, l, 11, MPI_COMM_WORLD);
                MPI_Send(&y[start], size, MPI_DOUBLE, l, 12, MPI_COMM_WORLD);
            }
        }
        for(int l = 0;l<p;l++){
            if(l != id){
                int pos = id == 0 ? data_chunk*l+mod_part:data_chunk*l;
                int size = id == 0 ? data_chunk:data_chunk+mod_part;
                MPI_Recv(&x[pos], size, MPI_DOUBLE, l, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&y[pos], size, MPI_DOUBLE, l, 12, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        if(id == 0){
            for (j=0;j<N;j++)
            {
                fprintf(fp,"%.6lf %.6lf\n",x[j],y[j]);
            }
            fprintf(fp,"---\n");
        }
        // #pragma omp parallel shared(x,y,q,j)
        // {
        #pragma omp parallel for 
        for(j=start;j<end;j++){
                Fx[j] = 0;
                Fy[j] = 0;
            for(k=0;k<N;k++){
                if(k!=j){
                    if(x[j] <= 1000000 && x[k] <= 1000000 && y[j] <= 1000000 && y[k] <= 1000000 && x[j] >= -1000000 && x[k] >= -1000000 && y[j] >= -1000000 && y[k] >= -1000000){
                        ftemp = (q[j]*q[k]) /(pow((x[j]-x[k]),2.0)+ pow((y[j]-y[k]),2.0));
                        fx = ftemp*((x[j]-x[k]))/sqrt(pow(x[j]-x[k],2.0)+ pow((y[j]-y[k]),2.0));
                        fy = ftemp*((y[j]-y[k]))/sqrt(pow(x[j]-x[k],2.0)+ pow((y[j]-y[k]),2.0));
                        Fx[j] = Fx[j] + fx;
                        Fy[j] = Fy[j] + fy;
                    }
                }
            }
                Ax[j] =  Fx[j]/m[j]; // find a in time = t
                Ay[j] =  Fy[j]/m[j]; // find a in time = t
                dax[j] = d * Ax[j]; // find a in time = t + d
                day[j] = d * Ay[j]; // find a in time = t + d
                dvx[j] = d * vx[j]; // find vx in time = t + d
                dvy[j] = d * vy[j]; // find vy in time = t + d
        }
        // }

    }
    if(id == 0){
        fclose(fp);
    }
    MPI_Finalize();
    return 0;
}
