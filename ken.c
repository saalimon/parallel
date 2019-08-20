#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <omp.h>

int main(int argc, char *argv[])
{
    int nt, rank;    
    int p;
    int i,j,k;
    int rowA,rowB,colA,colB;
    FILE *matA;
    FILE *matB;
    FILE *result;
    float *matrixA,*matrixB,*matrixResult;
    float sum=0;
    matA = fopen(argv[1],"r");
    matB = fopen(argv[2],"r");
    clock_t startTime,endTime;
    
            
    printf("Enter number of theads : ");
    scanf("%d",&p);

    omp_set_num_threads(p);
    double time=0;
    if(!matA||!matB){
        printf("File Not Found!!\n");
    }else{
        printf("Start read file\n");
        
        fscanf(matA,"%d %d",&rowA,&colA);
        fscanf(matB,"%d %d",&rowB,&colB);
        
        printf("rowA =%d colA=%d\n",rowA,colA);
        printf("rowB =%d colB=%d\n",rowB,colB);
        
        
        //Allocate memory 
        matrixA = (float*)calloc(rowA*colA,sizeof(float));
        matrixB = (float*)calloc(rowB*colB,sizeof(float));
        matrixResult = (float*)calloc(rowA*colB,sizeof(float));
        
        
        printf("Start read matA\n");
        while (!feof(matA))
            for(i=0; i<rowA; i++){
                for(j=0; j<colA; j++){
                    fscanf(matA, "%f", &matrixA[(i*colA)+j]);
                }
            }
        printf("Read matA completed\n");
        
        //Read matrixB with transposing
        printf("Start read matB\n");
        while (!feof(matB))
            for(i=0; i<rowB; i++){
                for(j=0; j<colB; j++){
                    fscanf(matB, "%f", &matrixB[j*(rowB)+i]);
                }
            }
        printf("Read matB completed\n");
        
        //Close file
        fclose(matA);
        fclose(matB);


        startTime = clock();
        #pragma omp parallel private(i,j,k,sum)
        {
        for(i=0;i<rowA;i++){
            for(j=0; j < colB; j++){
                sum=0;
                for(k = 0 ; k < rowB; k++){
                    sum += matrixA[(i*colA)+k]*matrixB[(j*colA)+k];
                }
                matrixResult[(i*colB)+j] = sum;
            }
        }
        }
        endTime = clock();

        result = fopen(argv[3],"w+");
        fprintf(result,"%d %d",rowA,colB);
        fprintf(result, "\n");
        
        for(i=0;i<rowA;i++){
            for(j=0;j<colB;j++){
                fprintf(result, "%.1f ",matrixResult[(i*colB)+j]);
            }
            fprintf(result, "\n");
        }
        
        free(matrixResult);
        fclose(result);

        time = ((double)(endTime - startTime)/CLOCKS_PER_SEC);
        printf("Successful! , Let's see result file.\n");
        printf("Timings: %lf sec\n",time);
        
    }
    return 0;
}
