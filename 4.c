// Recursive code for Matrix Multiplication 
#include<stdio.h>
#include<stdlib.h>

const int MAX = 8000; 

void multiplyMatrixRec(int row1, int col1, float** A, 
					int row2, int col2, float** B, 
					float** C) 
{ 
	// Note that below variables are static 
	// i and j are used to know current cell of 
	// result matrix C[][]. k is used to know 
	// current column number of A[][] and row 
	// number of B[][] to be multiplied 
	static int i = 0, j = 0, k = 0; 

	// If all rows traversed. 
	if (i >= row1) 
		return; 

	// If i < row1 
	if (j < col2) 
	{ 
	if (k < col1) 
	{ 
		C[i][j] += A[i][k] * B[k][j]; 
		k++; 

		multiplyMatrixRec(row1, col1, A, row2, col2, 
											B, C); 
	} 

	k = 0; 
	j++; 
	multiplyMatrixRec(row1, col1, A, row2, col2, B, C); 
	} 

	j = 0; 
	i++; 
	multiplyMatrixRec(row1, col1, A, row2, col2, B, C); 
} 

// Function to multiply two matrices A[][] and B[][] 
void multiplyMatrix(int row1, int col1, float** A, 
					int row2, int col2, float** B) 
{ 
	if (row2 != col1) 
	{ 
		printf("Not Possible\n"); 
		return; 
	} 

	float **C =(float**)  malloc(row1*sizeof(float*));
    for(int i = 0 ; i < row1 ; i++){
        C[i] = (float*) malloc(col2*sizeof(float));
    }

	multiplyMatrixRec(row1, col1, A, row2, col2, B, C); 

	// Print the result 
	for (int i = 0; i < row1; i++) 
	{ 
		for (int j = 0; j < col2; j++) 
			printf("%.1f ", C[i][j]); 

		printf("\n"); 
	} 
} 

// Driven Program 
int main(int argc,char** argv)
{ 
    float **matA;
    float **matB;
    int rowA;
    int rowB;
    int colA;
    int colB;
    float num;
    FILE *fp;
    int i,j,k,id,p;
    double startTime,endTime;  
        fp = fopen(argv[1], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&rowA,&colA);
            for(i = 0 ; i < rowA ; i++){
                matA[i] = (float*) malloc(colA*sizeof(float));
                for(j = 0 ; j < colA ; j++){
                    fscanf(fp,"%f ",&num);
                    matA[i][j] = num;
                }
            }            
            fclose(fp);
        }
        fp = fopen(argv[2], "r");
        if (fp) {
            fscanf(fp,"%d %d\n",&rowB,&colB);
            for(i = 0; i < rowB ; i++){
                matB[i] = (float*) malloc(colB*sizeof(float));
                for(j = 0 ; j < colB ; j++){
                    fscanf(fp,"%f ",&num);
                    matB[i][j] = num;
                }
            }
            fclose(fp);
        }
	multiplyMatrix(rowA, colA, matA, rowB, colB, matB); 
    printf("Timings :%f Sec\n",endTime-startTime);
    return 0;
} 

