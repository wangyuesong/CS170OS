#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

typedef struct matrix
{
	int rows;
	int cols;
	double* data;

} matrix; 

struct arg_struct
{
	int id;
	int start_row_index;
	int size_of_row;

	double *data_A;
	double *data_B;

	int rows_A;
	int cols_A;
	int rows_B;
	int cols_B;

}; 


struct result_struct
{
	int start_row_index;
	int size_of_row;
	double* data;
};

void *MultiThread(void *arg)
{
	struct result_struct *result;
	struct arg_struct *my_arg;

	my_arg = (struct arg_struct*)arg;
	result = (struct result_struct *)malloc(sizeof(struct result_struct));

	int start_row_index;
	int size_of_row;
	int id;
	double *data_A;
	double *data_B;
	int rows_A;
	int rows_B;
	int cols_A;
	int cols_B;

	start_row_index = my_arg->start_row_index;
	size_of_row = my_arg->size_of_row;
	id = my_arg->id;
	data_A = my_arg->data_A;
	data_B = my_arg->data_B;
	rows_A = my_arg->rows_A;
	rows_B = my_arg->rows_B;
	cols_A = my_arg->cols_A;
	cols_B = my_arg->cols_B;

	printf("Thread %d is running!\n",my_arg->id);
	fflush(stdout);

	free(my_arg);

	double *data_C;

	data_C = (double *)malloc(sizeof(double)*cols_B*size_of_row);
	
	int i, j, k;
	for (i = 0; i < size_of_row; i++)
	{
		for (j = 0; j < cols_A; j++)
		{
			for (k = 0; k < cols_B; k++)
			{
				data_C[i*(cols_B) + k]
					= data_A[(i + start_row_index)*(cols_A) + j]
					* data_B[j*(cols_B) + k];
			}
		}
	}

	result->data = data_C;
	return (void *)result;

}

matrix* composeMat(char *fileName)
{

	FILE *fp;
	char str[1000];

	int my_rows, my_cols;
	double *my_data;
	matrix *result;

	int count=0;
	int i, j;

	printf("Done!\n");
	fflush(stdout);

	result = (matrix *)malloc(sizeof(matrix));
	
	printf("Done!\n");
	fflush(stdout);

	/* opening file for reading */
	fp = fopen(fileName, "rt");

	if (fp == NULL)
	{
		perror("Error opening file!");
		exit(-1);
	}

	fgets(str, 1000, fp);

	sscanf(str, "%d  %d", &my_rows, &my_cols);

	if (my_rows <= 0 || my_cols <= 0)
	{
		printf("Matrix dimensions not valid!");
		exit(-1);
	}

	my_data = (double *)malloc(my_rows*my_cols*sizeof(double));

	char start;
	char rest[1000];

	while (fgets(str, 1000, fp) != NULL)
	{
		
		sscanf(str, "%c%s", &start, rest);

		if (start!='#')
		{
			i = count / my_cols;
			j = count % my_cols - 1;

			my_data[i*my_cols + j] = atof(str);
			//data_A[i*rows_A + j] = temp;
			//free(str);

			printf("%f\n", my_data[i*my_cols + j]);
			fflush(stdout);

			count++;
		}
	}

	printf("\n");

	fclose(fp);
	fp=NULL;

	result->cols = my_cols;
	result->rows = my_rows;
	result->data = my_data;

	return result;
}

int main(int argc, char **argv)
{
	int err;
	int thread_count;
	int t;
	matrix *MatA, *MatB;

	thread_count = atoi(argv[1]);

	MatA = composeMat("MatA.txt");
	
	printf("Done!\n");
	fflush(stdout);

	MatB = composeMat("MatB.txt");

	if (MatA->cols != MatB->rows)
	{
		printf("Matrix dimension not valid for multiply!");
		exit(-1);
	}

	struct arg_struct *args;
	struct result_struct *result;
	double *result_data;

	result_data = (double *)malloc(MatA->rows*MatB->cols*sizeof(double));

	pthread_t *thread_ids;
	thread_ids = (pthread_t *)malloc(thread_count*sizeof(pthread_t));

	int index = 0;
	int size_of_row = MatA->rows / thread_count;
	int res = MatA->rows % thread_count;

	printf("%d\n", thread_count);

	for (t = 0; t < thread_count; t++)
	{
		args = (struct arg_struct*)malloc(sizeof(struct arg_struct));
		args->id = (t + 1);
		args->start_row_index = index;
		args->size_of_row = size_of_row;
		if (res > 0)
		{
			args->size_of_row += 1;
			res--;
		}
		index = index + args->size_of_row;

		args->cols_A = MatA->cols;
		args->cols_B = MatB->cols;
		args->rows_A = MatA->rows;
		args->rows_B = MatB->rows;

		args->data_A = MatA->data;
		args->data_B = MatB->data;


		printf("Thread %d is creating!\n",args->id);
		fflush(stdout);

		err = pthread_create(&(thread_ids[t]), NULL, MultiThread, (void *)args);

		free(args);
	}

	int i;
	int j;
	for (t = 0; t < thread_count; t++)
	{
		result = (struct result_struct*)malloc(sizeof(struct result_struct));

		err = pthread_join(thread_ids[t], (void **)&result);

		for (i = 0; i < result->size_of_row; i++)
		{
			for (j = 0; j < MatB->cols; j++)
			{
				result_data[(i + result->start_row_index)*(MatB->cols) + j]
					= result->data[i*MatB->cols + j];
			}
		}

		free(result);
	}

	//for (i = 0; i < MatA->rows; i++)
	//{
	//	for (j = 0; j < MatB->cols; j++)
	//	{
	//		printf("(%d,%d): %f\n", result_data[i*MatB->cols + j]);
	//	}
	//}

	return 0;
}