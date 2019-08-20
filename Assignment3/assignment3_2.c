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
    printf("hello world\n");
    // // if(id ==0){
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
    // // }
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
    double ftemp;
    double fx;
    double fy;
    fp = fopen(argv[2], "w");
    for(i = 0;i<K+1;i++){
        for (j=0;j<N;j++)
        {
            //new speed vx and vy
            vx[j] = vx[j]+dax[j];
            vy[j] = vy[j]+day[j];
            //new location
            x[j] = x[j] + dvx[j];
            y[j] = y[j] + dvy[j];
            //output
            fprintf(fp,"%.6lf %.6lf\n",x[j],y[j]);
            if(j==0)printf("%.6lf %.6lf\n",dvx[j],dvy[j]);
        }
        fprintf(fp,"---\n");
    //     // printf("---\n");
        for(j=0;j<N;j++){
            Fx[j] = 0;
            Fy[j] = 0;
            for(k=0;k<N;k++){
                if(k!=j){
                    ftemp = (q[j]*q[k]) /(pow((x[j]-x[k]),2.0)+ pow((y[j]-y[k]),2.0));
                    fx = ftemp*((x[j]-x[k]))/sqrt(pow(x[j]-x[k],2.0)+ pow((y[j]-y[k]),2.0));
                    fy = ftemp*((y[j]-y[k]))/sqrt(pow(x[j]-x[k],2.0)+ pow((y[j]-y[k]),2.0));
                    Fx[j] = Fx[j] + fx;
                    Fy[j] = Fy[j] + fy;
                }
            }
            // printf("f = %lf , %lf\n",Fx[j],Fy[j]);
            Ax[j] =  Fx[j]/m[j]; // find a in time = t
            Ay[j] =  Fy[j]/m[j]; // find a in time = t
            dax[j] = d * Ax[j]; // find a in time = t + d
            day[j] = d * Ay[j]; // find a in time = t + d
            dvx[j] = d * vx[j]; // find vx in time = t + d
            dvy[j] = d * vy[j]; // find vy in time = t + d
            
            // dx[j] = dx[j] + dax[j] * (x[j]-x[k]) / sqrt(pow((x[j]-x[k]),2.0)+ pow((y[j]-y[k]),2.0));
            // dy[j] = dy[j] + day[j] * (y[j]-y[k]) / sqrt(pow((x[j]-x[k]),2.0)+ pow((y[j]-y[k]),2.0));
            // printf("%lf %lf\n",dvx[j],dvy[j]);
        }
       
    }
    fclose(fp);
    return 0;
}
