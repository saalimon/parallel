#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

void openRowCol(char file_name[],int count[]);
void openFile(char file_name[],int count[],float input[]);
void writeFile(char file_name[],int row,int col,float input[]);

int main(int argc, char *argv[]){
	int id,p;
	int i,j,k=0;
	int sep,diff;
	int temp;
	int row1,col1,row2,col2,inc;
	float *input1,*input2,*output,*output_total;
	int count1[2],count2[2],total[2];
	double StartTime,EndTime;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	
	if(id == 0){
		
		openRowCol(argv[1],count1);
		openRowCol(argv[2],count2);
		sep = (count1[0]*count2[1])/p;
		diff = (count1[0]*count2[1])%p;
		if(diff != 0)
			sep++;
		input1 = (float*)malloc(count1[0]*count1[1]*sizeof(float));
		input2 = (float*)malloc(count2[0]*count2[1]*sizeof(float));
		output = (float*)malloc(sep*sizeof(float));
		
		//Open file
		openFile(argv[1],count1,input1);
		openFile(argv[2],count2,input2);
		StartTime = MPI_Wtime();
	}
	
	//send row & col to each process
	output_total = (float*)malloc(count1[0]*count2[1]*sizeof(float));
	if(p > 1){
		MPI_Bcast(count1, 2, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(count2, 2, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&sep, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&diff, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}
	
	
	if(id > 0) {
		input1 = (float*)malloc(count1[0]*count1[1]*sizeof(float));
		input2 = (float*)malloc(count2[0]*count2[1]*sizeof(float));
		output = (float*)malloc(sep*sizeof(float));	
	}	
	//send data to each process
	if(p > 1){
		MPI_Bcast(input1, count1[0]*count1[1], MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(input2, count1[0]*count1[1], MPI_INT, 0, MPI_COMM_WORLD);
	}
	
	
	//master calculate
	if(id == 0){
		//yourself calculate
		for(i = 0 ; i < count1[0] && k < sep; i++){
			for(j = 0 ; j < count2[1]; j++){
				//find point
				if(k < sep){
					output[k] = 0;
					for(inc = 0 ; inc < count1[1] ; inc++){
						output[k] += input1[i*count1[1]+inc]*input2[j+inc*count2[1]];
					}
					k++;
				}
			}
		}
	}
	else { //slave calulate
		int startRow = (sep*id)/count2[1], startCol = (sep*id)%count2[1]-id+1;
		int check = 0;
		k = 0;
		temp = sep;
		if(id >= diff) temp--;
		for(i = startRow ; i < count1[0] && k <= temp+1; i++){
			for(j = 0 ; j < count2[1]; j++){
				if(check == 0){
					j = startCol;
					check = 1;
				}
					
				//find point
				if(k <= temp){
					output[k] = 0;
					for(inc = 0 ; inc < count1[1] ; inc++){
						output[k] += input1[i*count1[1]+inc]*input2[j+inc*count2[1]];
					}
					k++;
				}
			}
		}
		
	}
	//receive each process
	if(p > 1){
		int Displs[p];
		int tsep[p];
		temp = sep;
		Displs[0] = 0;
		tsep[0] = sep;
		if(diff != 0) diff--;
		for(i = 1; i < p; i++){
			Displs[i] = Displs[i-1] + temp;
			tsep[i] = sep;
			temp = sep;
			if(diff < i){
				temp--;
			}
		}
		MPI_Gatherv(output, sep, MPI_FLOAT, output_total,tsep,Displs, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}

	if(id==0){
		if(p == 1) output_total = output;
		EndTime = MPI_Wtime();
		printf("\nTime : %.7lf s\n",EndTime-StartTime);
		writeFile(argv[3],count1[0],count2[1],output_total);
		
	}
	MPI_Finalize();
	return 0;
}

void openRowCol(char file_name[],int count[]){
	FILE *fptr;
	if((fptr = fopen(file_name,"r")) != NULL) {
		fscanf(fptr,"%d %d ",&count[0],&count[1]);
	}
	fclose(fptr);
	
}

void openFile(char file_name[],int count[],float input[]){
	FILE *fptr;
	int i;
	if((fptr = fopen(file_name,"r")) != NULL) {
		fscanf(fptr,"%d %d\n",&count[0],&count[1]);
		for(i=0;i < count[0]*count[1];i++){
			fscanf(fptr,"%f",&input[i]);
		}
	}
	fclose(fptr);
}

void writeFile(char file_name[],int row,int col,float output[]){
	FILE *fp; 
	int i,j;
	fp = fopen(file_name,"w");
	if(fp != NULL){
		fprintf(fp,"%d %d\n",row,col); 
		for(i=0;i<row;i++){
			for(j=0;j<col;j++){
				fprintf(fp,"%.1lf ",output[i*col+j]);
			}
			fprintf(fp,"\n");			
		}
		fclose(fp);
	}
}
