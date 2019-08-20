/* Message passing skeleton for MPI N-Queens program.

   Author:    Timothy J. Rolfe
   Language:  C
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include "cpuTimes.h"

long int Nunique = 0,   // Accumulate results here
         Ntotal  = 0;

#define  INIT   1  // Message to client:  size and [0]
#define  DATA   2  // Message from client with results
#define  EXIT   4  // Message from client with CPU time
                   // Also to client, giving permission to exit
#define  TRACE  0  // Enable/disable tracing execution
/*
Here are the definitions of the public fields of MPI_Status:
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;
*/

// Server process:  send jobs to client compute engines and
// receives results back.
void StartQueens (int Size, double *clientTime)
{  int   col, k,
         commBuffer[2], // Communication buffer -- size, [0]
         Count[2],      // Counts back from the clients
         limit = (Size+1) / 2,  // Mirror images done automatically
         nProc,         // Size of the communicator
         proc,          // For loop [1..nProc-1] within initial message
         nActive;       // Number of active processes
   MPI_Status Status;

   if (TRACE) puts("Server process has entered StartQueens");
   MPI_Comm_size (MPI_COMM_WORLD, &nProc);
   commBuffer[0] = Size;
// Send initial configurations to all client processes --- or to those
// needed in case not all are required.
   for ( col = 0, proc = 1; proc < nProc && col < limit; proc++, col++ )
   {  commBuffer[1] = col;
      if (TRACE) printf ("Sending client %d job %d,%d\n", proc,
                         commBuffer[0], commBuffer[1]);
      MPI_Send (commBuffer, 2, MPI_INT, proc, INIT, MPI_COMM_WORLD);
   }
   nActive = proc-1;    // Since rank==0 is not used
   if (proc < nProc)    // More processes than jobs
   {  int dmy[2] = {0, 0};   // Termination message to unused processes
      while (proc < nProc)
         MPI_Send (dmy, 2, MPI_INT, proc++, INIT, MPI_COMM_WORLD);
   }
   if (TRACE) puts ("Server beginning to wait on results");
// Receive back results and send out new problems
   while ( col < limit )
   {  MPI_Recv(Count, 2, MPI_INT, MPI_ANY_SOURCE , DATA,
               MPI_COMM_WORLD, &Status);
      proc = Status.MPI_SOURCE;
      if (TRACE) printf ("Received results from client %d (%d, %d)\n",
                 proc, Count[0], Count[1]);
      Nunique += Count[0];
      Ntotal  += Count[1];
      commBuffer[1] = col++;
      if (TRACE) printf ("Sending client %d job %d,%d\n", proc,
                         commBuffer[0], commBuffer[1]);
      MPI_Send (commBuffer, 2, MPI_INT, proc, INIT, MPI_COMM_WORLD);
   }
// Finally, receive back pending results and send termination
// indication (message with size of zero).
   commBuffer[0] = 0;
   while (nActive > 0)
   {
      if (TRACE) printf ("%d pending\n", nActive);
      MPI_Recv(Count, 2, MPI_INT, MPI_ANY_SOURCE , DATA,
               MPI_COMM_WORLD, &Status);
      --nActive;
      proc = Status.MPI_SOURCE;
      if (TRACE) printf ("Received results from client %d (%d, %d)\n",
                 proc, Count[0], Count[1]);
      Nunique += Count[0];
      Ntotal  += Count[1];
      if (TRACE) printf ("Sending client %d termination message\n", proc);
      MPI_Send (commBuffer, 2, MPI_INT, proc, INIT, MPI_COMM_WORLD);
   }
   for (k = 1; k < nProc; k++)
   {  double time;
      if (TRACE) printf("Waiting for time #%d\n", k);
      MPI_Recv(&time, 1, MPI_DOUBLE, MPI_ANY_SOURCE , EXIT,
               MPI_COMM_WORLD, &Status);
      proc = Status.MPI_SOURCE;
      clientTime[proc] = time;
      if (TRACE) printf("Client %d sent time %3.3lf\n", proc, time);
   }
   for (proc = 1; proc < nProc; proc++)
   {  MPI_Send(&proc, 0, MPI_INT, proc, EXIT, MPI_COMM_WORLD);
      if (TRACE) printf ("Sending EXIT to %d\n", proc);
   }
   if (TRACE) puts("Exiting StartQueens.");
}

// Macro to swap datatype x, locations y and z
#define  swap(x,y,z) { x temp = y; y = z; z = temp;  }

// Client processes receive problems to process from the
// server and then return their results to the server.
void ProcessQueens(int myPos)
{
   int  nCells = 0, size, k, col,
        buffer[2];
   double startTime, endTime, elapsed;
   MPI_Status Status;

   if (TRACE) printf("Client %d has entered ProcessQueens.\n", myPos);
   startTime = cpuClock();
   MPI_Recv(buffer, 2, MPI_INT, 0, INIT, MPI_COMM_WORLD, &Status);
   if (TRACE) printf("Client %d has received problem: %d and %d\n",
                     myPos, buffer[0], buffer[1]);
   size = buffer[0];
   col  = buffer[1];
// Logic goes here to initialize fields for problem solving.

// As long as a valid problem is in hand, do the processing.
// The server sends a size of zero as a termination message
   while (size > 0)
   {  int Count[2];
   // Zero out the counters for THIS problem start.
      Nunique = 0,
      Ntotal  = 0;
   // Logic for setting up for this specific problem
   // Send back random data
      Count[0] = rand() % 10;
      Count[1] = Count[0]*4 + rand()%10;
      if (TRACE) printf("Client %d sending results (%d, %d).\n",
                        myPos, Count[0], Count[1]);
      MPI_Send(Count,  2, MPI_INT, 0, DATA, MPI_COMM_WORLD);
   // Get the next job --- or the termination message.
      if (TRACE) printf("Client %d waiting for job,\n", myPos);
      MPI_Recv(buffer, 2, MPI_INT, 0, INIT, MPI_COMM_WORLD, &Status);
      size = buffer[0];
      col  = buffer[1];
   }
// Return the total CPU time required.
   endTime = cpuClock();
   elapsed = endTime - startTime;
   if (TRACE) printf("Client %d sending time %3.3lf.\n", myPos, elapsed);
   MPI_Send(&elapsed, 1, MPI_DOUBLE, 0, EXIT, MPI_COMM_WORLD);
// Final hand-shake:  get permission to terminate
   MPI_Recv(buffer, 0, MPI_INT, 0, EXIT, MPI_COMM_WORLD, &Status);
}

main(int argc, char *argv[])
{
   int    nProc,                    // Processes in the communicator
          proc;                     // loop variable
   MPI_Status Status;               // Return status from MPI
   int    rc;                       // Status  code from MPI_Xxx() call
   int    myPos;                    // My own position

   rc = MPI_Init(&argc, &argv);
   if (rc != MPI_SUCCESS)
   {  puts ("MPI_Init failed"); exit(-1);  }

   rc = MPI_Comm_rank (MPI_COMM_WORLD, &myPos);
   rc = MPI_Comm_size (MPI_COMM_WORLD, &nProc);
   if (TRACE)
   {
      printf ("Process %d of %d started.\n", myPos, nProc);
      fflush (stdout);
   }
   if ( myPos == 0 )    // I.e., this is the server/master/host
   {
      int    Size,
             k;
      FILE  *fptr;
      double ClockT, CPU[2], Clock[2], Lapsed,
            *clientTime = (double*) calloc(nProc, sizeof *clientTime);

#ifdef DEBUG
      puts("Server has entered its part of main");
#endif
      if (argc < 2)
      {
         fputs ("Size:  ", stdout);
         scanf ("%d", &Size);
      }
      else
      {
         Size = atoi(argv[1]);
      }

      getTimes(Clock, CPU);
#ifdef DEBUG
      puts ("Server is calling StartQueens.");
#endif
      StartQueens (Size, clientTime);
#ifdef DEBUG
      puts ("Server is back from StartQueens.");
#endif
      getTimes(&Clock[1], &CPU[1]);
      Lapsed = CPU[1] - CPU[0];
      ClockT = Clock[1] - Clock[0];
      printf ("%3d ==> %10ld  %10ld  %15.8lg  %15.8lg\n",
              Size, Nunique, Ntotal, Lapsed, ClockT);
      for (k = 1; k < nProc; k++)
         printf ("%15.7lg", clientTime[k]);
      putchar('\n');
   }
   else  // I.e., this is the client/slave/node
      ProcessQueens(myPos);
#ifdef DEBUG
   printf ("Process %d is back\n", myPos);
#endif
   MPI_Finalize();
   exit(0);
}
