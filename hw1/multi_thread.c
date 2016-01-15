#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

int debug = 1;
typedef struct {
    int rows;
    int cols;
    double *data;
} matrix;

typedef struct {
    matrix * a;
    matrix * b;
    //from inclusive, to exclusive
    int from_row;
    int to_row;
} in_arg;

typedef struct{
    double * result;
    int from_row;
    int to_row;
} out_arg;

double CTimer() {
    struct timeval tm;
    
    gettimeofday(&tm,NULL);
    return((double)tm.tv_sec + (double)(tm.tv_usec/1000000.0));
}


int isNumeric(const char *str){
    int decimal = 0;
    if(*str == '-') ++str;
    else if(*str == '.'){
        decimal = 1;
        ++str;
    }
    if(!*str) return 0;
    while(*str){
        if(!decimal && *str == '.'){
            decimal = 1;
            ++str;
        }
        if(!isdigit(*str)) return 0;
        else ++str;
    }
    return 1;
}

matrix * read_matrix(char * loc){
    FILE *fp;
    matrix * result_matrix;
    if((fp=fopen(loc,"rt")) == NULL){
        printf("ERROR: Cannot find the file %s\n", loc);
        exit(-1);
    }
    int row = -1;
    int col = -1;
    char *row_col = (char*)malloc(60* sizeof(char));
    if(fgets(row_col, 60, fp) != NULL){
        if(sscanf(row_col, "%d%d", &row, &col) !=2){
            printf("ERROR: Matrix dimensional count incorrect\n");
            exit(-1);
        }
    }
    else{
        printf("ERROR: Matrix dimensional count incorrect\n");
        exit(-1);
    }
    result_matrix = (matrix *)malloc(sizeof(matrix));
    result_matrix->rows = row;
    result_matrix->cols = col;
    
    result_matrix->data = (double *)malloc(row * col * sizeof(double));
    for(int i = 0; i < row; i ++){
        for(int j = 0; j < col; j ++){
            char line[1000];
            memset(line,0,sizeof(line));
            while(fgets(line, 1000, fp)!= NULL){
                if(line[0] == '#')
                    continue;
                else
                    break;
            }
            if(!isNumeric(line)){
                fprintf(stderr,"Error while parsing line: %s\n", line);
                exit(-1);
            }
            result_matrix->data[i * col + j] = atof(line);
            if(debug){
                printf("\n");
                printf("Current String:%s\n",line);
                printf("Current Double:%f\n", result_matrix->data[i * col + j]);
            }
            //        free(line);
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
    double * result = (double *)malloc((arg->to_row - arg->from_row) * arg->b->cols * sizeof(double));
    out_arg * out = (out_arg *)malloc(sizeof(out_arg));
    out->result = result;
    
    for(int i = arg->from_row; i < arg->to_row; i ++){
        int result_start = (i - arg->from_row) * arg->b->cols;
        double * one_strip = compute_strip(arg->a, arg->b, i);
        //Copy to result
        for(int j = 0; j < arg->b->cols; j++){
            result[result_start + j] = one_strip[j];
        }
        free(one_strip);
    }
    
    out->from_row = arg->from_row;
    out->to_row = arg->to_row;
    return (void *)out;
}

int main(int argc, char ** argv){
    double start_time = CTimer();
    matrix * matrix_a;
    matrix * matrix_b;
    int thread_count = 0;
    if(!debug){
        if(argc != 7){
            fprintf(stderr, "%s\n", "Not enough arguments");
            exit(-1);
        }
    }

    int optValue=0;
    while((optValue = getopt(argc,argv,"a:b:t")) != EOF) {
        switch(optValue) {
            case 'a':
                matrix_a = read_matrix(optarg);
                break;
            case 'b':
                matrix_b = read_matrix(optarg);
                break;
            case 't':
                thread_count = atoi(optarg);
                break;
            default:
                fprintf(stderr,"Option not recognized %c\n", (char)optValue);
                fprintf(stderr,"Usage: %s","my_matrix_multiply -a a_matrix_file.txt -b b_matrix_file.txt -t thread_count\n");
                exit(1);
        }
    }

    if (thread_count<1){
        fprintf(stderr, "%s", "Thread count must be >=1 \n");
        exit(-1);
    }
    // for(int i = 1; i < argc; i ++){
    //     if(strcmp(argv[i],"-a") == 0){
    //         matrix_a = read_matrix(argv[i+1]);
    //     }
    //     if(strcmp(argv[i],"-b") == 0){
    //         matrix_b = read_matrix(argv[i+1]);
    //     }
    //     if(strcmp(argv[i],"-t") == 0){
    //         thread_count = atoi(argv[i+1]);
    //     }
    // }
    if(matrix_a->cols != matrix_b->rows){
        fprintf(stderr, "%s", "a and b not computable\n");
        exit(-1);
    }
    
    int err;
    int left_over =  matrix_a->rows % thread_count;
    int average = matrix_a->rows / thread_count;
    int row_count = 0;
    pthread_t thread_ids[thread_count];
    
    double * final_result = (double *)malloc(matrix_a->rows * matrix_b->cols * sizeof(double));
    if(thread_count > matrix_a->rows){
        thread_count = matrix_a->rows;
    }
    for(int i = 0; i < thread_count; i ++){
        in_arg * arg = (in_arg *)malloc(sizeof(in_arg));
        arg->a = matrix_a;
        arg->b = matrix_b;
        arg->from_row = row_count;
        if(left_over != 0){
            row_count = row_count + average + 1;
            arg->to_row = row_count;
            left_over --;
        }
        else{
            row_count += average;
            arg->to_row = row_count;
        }
        err = pthread_create(&(thread_ids[i]), NULL, compute_thread, (void *)arg);
        if(debug)
            printf("Thread %d started, take row %d to %d \n",i,arg->from_row, arg->to_row);
    }
    
    for(int i = 0; i < thread_count; i ++){
        out_arg * out = (out_arg *)malloc(sizeof(out_arg));
        if(debug)
            printf("Main thread is about to join threads\n");
        err = pthread_join(thread_ids[i], (void**)&out);
        double *result = out->result;
        for(int i = out->from_row; i < out->to_row; i++){
            for(int j = 0; j < matrix_b->cols; j ++){
                final_result[i*matrix_b->cols + j] = result[(i - out->from_row) * matrix_b->cols + j];
            }
        }
        free(out);
        free(result);
        if(debug)
            printf("Thread %d joined\n",i);
    }
    if(debug){
        printf("Total time spent:%f\n", CTimer() - start_time);
        printf("\n");
    }
    printf("%d %d\n", matrix_a->rows, matrix_b->cols);
    for(int i = 0 ; i < matrix_a->rows * matrix_b->cols; i ++){
        if(i % matrix_b-> cols == 0)
            printf("# Row %d\n",i/ matrix_b->cols);
        printf("%f\n",final_result[i]);
        if(i % matrix_b->cols == matrix_b->cols-1)
            printf("\n");
    }
    
    
}





