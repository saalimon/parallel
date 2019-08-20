#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void test_system
 ( int n, double **A, double *b );
/*
 * Given n on entry,
 * On return is an n-by-n matrix A
 * with n+1 on the diagonal and 1 elsewhere.
 * The elements of the right hand side b
 * all equal 2*n, so the exact solution x
 * to A*x = b is a vector of ones. */

void run_jacobi_method
 ( int id, int p,
   int n, double **A, double *b,
   double epsilon, int maxit,
   int *numit, double *x );
/*
 * Runs the Jacobi method for A*x = b.
 *
 * ON ENTRY :
 *   id       processor identification label;
 *   p        number of processors;
 *   n        the dimension of the system;
 *   A        an n-by-n matrix A[i][i] /= 0;
 *   b        an n-dimensional vector;
 *   epsilon  accuracy requirement;
 *   maxit    maximal number of iterations;
 *   x        start vector for the iteration.
 *
 * ON RETURN :
 *   numit    number of iterations used;
 *   x        approximate solution to A*x = b. */

int main ( int argc, char *argv[] )
{
   int n,i,np,id;

   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&id);
   MPI_Comm_size(MPI_COMM_WORLD,&np);

   if(argc > 1)
      n = atoi(argv[1]);
   else
   {
      printf("give the dimension : ");
      scanf("%d",&n);
   }
   { /* every node generates the test system */
      double *b;
      b = (double*) calloc(n,sizeof(double));
      double **A;
      A = (double**) calloc(n,sizeof(double*));
      for(i=0; i<n; i++)
         A[i] = (double*) calloc(n,sizeof(double));
      test_system(n,A,b);
      double *x;
      x = (double*) calloc(n,sizeof(double));
      /* we start at an array of all zeroes */
      for(i=0; i<n; i++) x[i] = 0.0;
      double eps = 1.0e-4;
      int maxit = 2*n*n;
      int cnt = 0;
      run_jacobi_method(id,np,n,A,b,eps,maxit,&cnt,x);
      if(id == 0)
      {
         printf("computed %d iterations\n",cnt);
         double sum = 0.0;
         for(i=0; i<n; i++) /* compute the error */
         {
            double d = x[i] - 1.0;
            sum += (d >= 0.0) ? d : -d;
         }
         printf("error : %.3e\n",sum);
      }
   }
   MPI_Finalize();
   return 0;
}

void test_system
 ( int n, double **A, double *b )
{
   int i,j;
   for(i=0; i<n; i++)
   {
      b[i] = 2.0*n;
      for(j=0; j<n; j++) A[i][j] = 1.0;
      A[i][i] = n + 1.0;
   }
}

void run_jacobi_method
 ( int id, int p,
   int n, double **A, double *b,
   double epsilon, int maxit,
   int *numit, double *x )
{
   double *dx,*y;
   dx = (double*) calloc(n,sizeof(double));
   y = (double*) calloc(n,sizeof(double));
   int i,j,k;
   double sum[p];
   double total;
   int dnp = n/p;
   int istart = id*dnp;
   int istop = istart + dnp;

   for(k=0; k<maxit; k++)
   {
      sum[id] = 0.0;
      for(i=istart; i<istop; i++)
      {
         dx[i] = b[i];
         for(j=0; j<n; j++)
            dx[i] -= A[i][j]*x[j]; 
         dx[i] /= A[i][i];
         y[i] += dx[i];
         sum[id] += ( (dx[i] >= 0.0) ? dx[i] : -dx[i]);
      }
      for(i=istart; i<istop; i++) x[i] = y[i];
      MPI_Allgather(&x[istart],dnp,MPI_DOUBLE,x,dnp,
                    MPI_DOUBLE,MPI_COMM_WORLD);
      MPI_Allgather(&sum[id],1,MPI_DOUBLE,sum,1,
                    MPI_DOUBLE,MPI_COMM_WORLD);
      total = 0.0;
      for(i=0; i<p; i++) total += sum[i];
      if(id == 0) printf("%3d : %.3e\n",k,total);
      if(total <= epsilon) break;
   }
   *numit = k+1;
   free(dx);
}