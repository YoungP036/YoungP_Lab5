#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
void get_row(int ncols, int row, char *input,double *ret);
double *get_row2(int ncols, int row, char *input);
void get_col(int nrows, int ncols,int col, char *file, double *ret);
int get_ncols(char *input);
int get_nrows(char *input);
int get_row_from_linear_index(int index, int ncols);
int get_col_from_linear_index(int index, int ncols);
int get_linear_index_from_mIndex(int row, int col, int ncols, int nrows);

int main(int argc, char *argv[])
{
	char* m1 = "mat1.txt";
	char* m2 = "mat2.txt";
	int nrowsA=get_nrows(m1);
	int nrowsB=get_nrows(m2);
	int ncolsA=get_ncols(m1);
	int ncolsB=get_ncols(m2);
	int i,j,k;
	int myid, numprocs;
	MPI_Status status;
	int master=0;
	double starttime, endtime;
	int m_tag,s_tag,s2_tag;
	int col_from_index,row_from_index;
	int ans_row_index, sender_proc;
	int current_proc;

	double* curr_row=(double*)malloc(sizeof(double)*ncolsB);
	double* curr_col=(double*)malloc(sizeof(double)*nrowsA);
	double* s_row = (double*)malloc(sizeof(double)*ncolsB);
	double* s_col=(double*)malloc(sizeof(double)*nrowsA);
	double **final_ans=(double**)malloc(sizeof(double*)*nrowsA);
	double *ans=(double*)malloc(sizeof(double)*nrowsA);
	for(i=0;i<nrowsA;i++)
		ans[i]=(double*)malloc(sizeof(double)*ncolsB);
	for(i=0;i<nrowsA;i++)
		for(j=0;j<ncolsB;j++)
			ans[i][j]=0.0;
	double* s_ans =(double*)malloc(sizeof(double)*nrowsA);
		for(i=0;i<nrowsA;i++)
			s_ans=0.0;
	
	//capture matrix 2
	double **matB=(double**)malloc(sizeof(double*)*nrowsB);
	for(i=0;i<nrowsB;i++)
		matB=(double*)malloc(sizeof(double)*ncolsB);
	for(i=0;i<nrowsB;i++)
		get_row(ncolsB,i+1,m2,matB[i]);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	current_proc=0;
	//master
	if(myid==0){
		starttime = MPI_Wtime();
		MPI_Bcast(&matB[0][0], nrowsA*ncolsB, MPI_DOUBLE, master, MPI_COMM_WORLD);
		//send the first numprocs number of rows
		for(i=0;i<min(numprocs-1,nrowsA);i++){
			for(j=0;j<nrowsA;j++)
				get_row(ncolsA, i+1, m1, curr_row);
				MPI_Send(curr_row,ncolsA, MPI_DOUBLE,i+1,i+1,MPI_COMM_WORLD);
				numsent++;
		}
		//send remaining rows and process recvs
		for(i=0;i<nrowsA;i++){
			MPI_Recv(&ans,nrowsA,MPI_DOUBLE,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			sender_proc=status.MPI_SOURCE;
			ans_row_index=status.MPI_TAG;
			final_ans[ans_row_index]=ans;
			if(numsent<nrowsA){
				get_row(ncolsA,numsent+1,m1,curr_row);
				MPI_Send(curr_row,ncolsA,MPI_DOUBLE,sender,numsent+1,MPI_COMM_WORLD);
				numsent++;
			}
			else
				MPI_SEND(MPI_BOTTOM,0,MPI_DOUBLE,sender,0,MPI_COMM_WORLD);
			
		}
		endtime=MPI_wtime();
		printf("%f\n",(endtime-starttime));

		//print final answer
		for(i=0;i<nrowsA;i++){
			printf("\n");
			for(j=0;j<ncolsB;j++)
				printf(" %f",ans[i][j]);
		}
		printf("\n");
		endtime = MPI_Wtime();
	}
//slave
	else{
		if(myid<nrowsA){		
			MPI_Bcast(&matB[0][0], nrowsA*ncolsB, MPI_DOUBLE, master, MPI_COMM_WORLD);
			while(1){
				MPI_Recv(&curr_row, ncolsB, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				if(status.MPI_TAG==0)
					break;
				int i;
				for(i=0;i<ncolsA;i++)
					for(k=0;k<ncolsB+1;k++)
						s_ans[i]+=curr_row[k]*curr_col[k];
				//SEND to master
				MPI_Send(ans,ncolsA,MPI_DOUBLE,master,status.MPI_TAG,MPI_COMM_WORLD);
			}
		}			
	}
	MPI_Finalize();
	return 0;
}//end main
int get_nrows(char *input)
{
	FILE *fp;
	fp=fopen(input, "r+");
	if(fp==NULL){
		printf("No file\n");
		return -1;
	}
	int row_count = 0;
	int c;
	
	while ((c = fgetc(fp)) != EOF)
		if (c == '\n')
			row_count++;
			
	fclose(fp);
	return row_count;
}

int get_ncols(char *input)
{
	FILE *fp;
	fp=fopen(input,"r+");
	if(fp==NULL){
		printf("No file\n");
		return -1;
	}

	int col_count = 1;
	int c;

	while ((c = fgetc(fp)) != '\n')
		if (c == ' ')
			col_count++;
	fclose(fp);
	return col_count;
}
void get_col(int nrows, int ncols,int col, char *file, double *ret)
{
	//file setup
	FILE *fp = fopen(file, "r");
	if (fp == NULL){
		printf("file not found\n");
		return -1;
	}

	double chr;
	int curr_col = 1;

	//iterate to col
	while (curr_col != col)
	{
		// printf("loop iter : %d\n",curr_col);
		fscanf(fp, "%lf", &chr);
		curr_col++;
	}

	int i, j;
	i = 0;
	j = 0;
	//capture col
	while (i != nrows)
	{
		fscanf(fp, "%lf", &chr);
		if (j % ncols == 0)
		{
			*ret = chr;
			ret++;
			i++;
		}
		j++;
	}
}
// double *get_row2(int ncols, int row, char *input)
// {
// 	//file setup
// 	FILE *fp;
// 	fp = fopen(input, "r");
// 	if (fp == NULL)
// 	{
// 		printf("No File Found");
// 		return -1;
// 	}

// 	//heap setup
// 	double *m;
// 	m = malloc(sizeof(double) * ncols);
// 	double *mm = m;
// 	if (m == NULL)	
// 		return -1;
	
// 	//stack setup
// 	double c;
// 	int i;
// 	int r = 1;

// 	//get to row
// 	while (r != row)
// 		if ((c = fgetc(fp)) == '\n')
// 			r++;
// 	//capture row
// 	for (i = 0; i < ncols; i++)
// 	{
// 		fscanf(fp, "%lf", &c);
// 		*mm = c;
// 		mm++;
// 	}
	

// 	fclose(fp);
// 	return m;
// }
void get_row(int ncols, int row, char *input,double *ret)
{
	//file setup
	FILE *fp;
	fp = fopen(input, "r");
	if (fp == NULL)
	{
		printf("No File Found");
		return -1;
	}
	
	double *m=ret;
	double c;
	int r = 1;
	int i;
	
	//iterate to correct row
	while (r != row){
		if ((c = fgetc(fp)) == '\n')
			r++;

	//capture row
	for (i = 0; i < ncols; i++)
	{
		fscanf(fp, "%lf", &c);
		*m = c;
		m++;
	}
	fclose(fp);
}

int get_row_from_linear_index(int index, int ncols){
	int row=0;
	while(index>=ncols){
		index-=ncols;
		row++;
	}
	return row;
}

int get_col_from_linear_index(int index, int ncols){
	return index%ncols;
}

int get_linear_index_from_mIndex(int row, int col, int nrows, int ncols){
	int index=0;
	index+=col;
	index+=row*ncols;
	return index;
}
