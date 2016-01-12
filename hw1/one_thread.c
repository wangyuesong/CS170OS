#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    int rows;
    int cols;
    double *data;
} matrix;

typedef struct {
    matrix * a;
    matrix * b;
    //Both inclusive
    int from_row;
    int to_row;
} in_arg;

typedef struct{
    double * result;
} out_arg;

matrix * read_matrix(char * loc){
    FILE *fp;
    matrix * result_matrix;
    if((fp=fopen(loc,"rt")) == NULL){
        printf("ERROR: Cannot find the file %s", loc);
        exit(-1);
    }
    int row = -1;
    int col = -1;
    char *row_col = (char*)malloc(60* sizeof(char));
    if(fgets(row_col, 60, fp) != NULL){
        if(sscanf(row_col, "%d%d", &row, &col) !=2){
            printf("ERROR: Matrix dimensional count incorrect");
            exit(-1);
        }
    }
    else{
        printf("ERROR: Matrix dimensional count incorrect");
        exit(-1);
    }
    result_matrix = (matrix *)malloc(sizeof(matrix));
    result_matrix->rows = row;
    result_matrix->cols = col;
    
    result_matrix->data = (double *)malloc(row * col * sizeof(double));
    for(int i = 0; i < row; i ++){
        for(int j = 0; j < col; j ++){
            char line[1000];
            while(fgets(line, 1000, fp)!= NULL){
                if(line[0] == '#')
                    continue;
                else
                    break;
            }
            result_matrix->data[i * col + j] = atof(line);
        }
    }
    
    return result_matrix;
}

double* compute_strip(matrix *a, matrix *b, int a_strip){
    
    double *strip = (double *)malloc(b->cols * sizeof(double));
    int a_base = a->cols * a_strip;
    //one b col
    for(int i = 0 ; i < b->cols; i ++){
        double result = 0;
        for(int j = 0 ; j < b->rows; j++){
            result += a->data[a_base + j] * b->data[i + j * b->cols];
        }
        strip[i] = result;
    }
    return strip;
}


void * compute_thread(void * a){
    in_arg * arg = (in_arg *)a;
    //一维模拟二维
    printf("Computing thread started");
    
    double * result = (double *)malloc((arg->to_row - arg->from_row) * arg->b->cols * sizeof(double));
    out_arg * out = (out_arg *)malloc(sizeof(out_arg));
    out->result = result;
    
    for(int i = arg->from_row; i <= arg->to_row; i ++){
        int result_start = (i - arg->from_row) * arg->b->cols;
        double * one_strip = compute_strip(arg->a, arg->b, i);
        //Copy to result
        for(int j = 0; j < arg->b->cols; j++){
            result[result_start + j] = one_strip[j];
        }
        free(one_strip);
    }
    
    printf("Computing thread finished");
    return (void *)result;
}

int main(int argc, char ** argv){
    matrix * matrix_a;
    matrix * matrix_b;
    int thread_count = 0;
    //  if(argc != 7){
    //      fprintf(stderr, "%s", "Not enough arguments");
    //      exit(-1);
    //  }
    for(int i = 1; i < argc; i ++){
        if(strcmp(argv[i],"-a") == 0){
            matrix_a = read_matrix(argv[i+1]);
        }
        if(strcmp(argv[i],"-b") == 0){
            matrix_b = read_matrix(argv[i+1]);
        }
        if(strcmp(argv[i],"-t") == 0){
            thread_count = atoi(argv[i+1]);
        }
    }
    if(matrix_a->cols != matrix_b->rows){
        fprintf(stderr, "%s", "a and b not computable");
        exit(-1);
    }
    printf("Read ready, start computing");

    
    pthread_t thread_id;
    int err;
    in_arg * arg = (in_arg *)malloc(sizeof(in_arg));
    out_arg * result = (out_arg *)malloc(sizeof(out_arg));
    arg->a = matrix_a;
    arg->b = matrix_b;
    arg->from_row = 0;
    arg->to_row = matrix_a->rows - 1;
       err = pthread_create(&thread_id, NULL, compute_thread, (void*)arg);
    
    err = pthread_join(thread_id, (void**)result);
//    double * result_matrix = (double *)malloc(matrix_a->rows * matrix_b->cols * sizeof(double));
//    for(int i = 0 ; i < matrix_a->rows; i++){
//        double *one_strip = compute_strip(matrix_a,matrix_b,i);
//        for(int j = 0 ; j < matrix_a->cols; j++){
//            result_matrix[i * matrix_b->cols + j] = one_strip[j];
//        }
//        free(one_strip);
//    }
    for(int i = 0 ; i < matrix_a->rows * matrix_b->cols; i ++){
        printf("%f ",result->result[i]);
        if(i % matrix_b->cols == matrix_b->cols-1)
            printf("\n");
    }
}





