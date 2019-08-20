#include<stdio.h>
#include<stdlib.h>
#include<math.h>
// #include<mpi.h>
// #include<omp.h> 
int main(int argc,char** argv)
{
    int i,j,k;
    FILE *fp;
    // MPI_Status status;
    // MPI_Init(NULL,NULL);
    // MPI_Comm_size(MPI_COMM_WORLD,&p);
    // MPI_Comm_rank(MPI_COMM_WORLD,&id); 
    int N,K;
    double d;
    double *m;
    double *q;
    double *x;
    double *y;
    double *vx;
    double *vy;
    double *f;
    double *a;
    double *da;
    double *dx;
    double *dy;
    double *dvx;
    double *dvy;
    // if(id ==0){
        fp = fopen(argv[1], "r");
        if(fp){
            fscanf(fp,"%d %d\n%lf\n",&N,&K,&d);
            m   = (double*) malloc(N*sizeof(double));
            q   = (double*) malloc(N*sizeof(double));
            x   = (double*) malloc(N*sizeof(double));
            y   = (double*) malloc(N*sizeof(double));
            vx  = (double*) malloc(N*sizeof(double));
            vy  = (double*) malloc(N*sizeof(double));        
            for( i = 0; i < N; i++)
            {
                fscanf(fp,"%lf %lf %lf %lf %lf %lf\n",&m[i],&q[i],&x[i],&y[i],&vx[i],&vy[i]);
            }
            fclose(fp);
        }
    // }
    f = (double*) calloc(N,sizeof(double));
    a = (double*) calloc(N,sizeof(double));
    da = (double*) calloc(N,sizeof(double));
    dx = (double*) calloc(N,sizeof(double));
    dy = (double*) calloc(N,sizeof(double));
    dvx = (double*) calloc(N,sizeof(double));
    dvy = (double*) calloc(N,sizeof(double));
    for(i = 0;i<K+1;i++){
        for(j=0;j<N;j++){
            for(k=0;k<N;k++){
                if(k!=j)
                    f[j] = (q[j]*q[k]) /(pow((x[j]-x[k]),2.0)+ pow((y[j]-y[k]),2.0));
            }
            a[j] =  f[j]/m[j]; // find a in time = t
            da[j] = d * a[j]; // find a in time = t + d
            dvx[j] = d * vx[j]; // find vx in time = t + d
            dvy[j] = d * vy[j]; // find vy in time = t + d
            for(k=0;k<N;k++){
                if(k!=j){
                    dx[j] = da[j] * (x[j]-x[k]) / sqrt(pow((x[j]-x[k]),2.0)+ pow((y[j]-y[k]),2.0));
                    dy[j] = da[j] * (y[j]-y[k]) / sqrt(pow((x[j]-x[k]),2.0)+ pow((y[j]-y[k]),2.0));
                }
            }
            // printf("%lf %lf\n",x[j],y[j]);
        }
        for (j=0;j<N;j++)
        {
            //new speed vx and vy
            vx[j] = vx[j]+dx[j];
            vy[j] = vy[j]+dy[j];
            //new location
            x[j] = x[j] + dvx[j];
            y[j] = y[j] + dvy[j];
            //output
            printf("%lf %lf\n",x[j],y[j]);
        }
        printf("---\n");
    }
    return 0;
}
