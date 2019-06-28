/*
 GBC_Linux_homework #2
 21800758 Choi Hayeong
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

void *ThreadFn_sum(void *param);

typedef struct _ThreadInfo{
    int threadIdx; //the index of current thread
    int noThread; //number of threads
    int *data; //the data array
    int size; //the size of the data array
    int result; //a variable to return the parial sum
} ThreadInfo;

/* compute the partial sum of the array */
void *ThreadFn_sum(void *param){
    int partial_sum = 0, i = 0;
    ThreadInfo *info = (ThreadInfo*)param;
    for(i = info->threadIdx; i < info->size; ){
        partial_sum += info->data[i];
        i += info->noThread;
    }
    usleep(100);
    info->result = partial_sum;
    
    printf("Partial sum of thread %d = %d\n", info->threadIdx, partial_sum);
    return 0;
}

int main(int argc, const char *argv[]){
    ThreadInfo *threadInfo;     //구조체 포인터
    pthread_t *tid; //스레드 ID
    int *data;
    int count = 0;  //input data
    long long int total_sum = 0;
    int size = atoi(argv[1]);    // 입력한 number ex.1000
    int noThread = atoi(argv[2]);    //# of thread ex.10
    int thr_id, result;     //check create(), join()
    
    if(argc < 2){
        fprintf(stderr, "usage: ./a.out <integer>\n");
        exit(0);
    }

    //malloc
    tid = (pthread_t *)malloc(sizeof(pthread_t) * noThread);
    threadInfo = (ThreadInfo *)malloc(sizeof(threadInfo) * noThread); //threadInfo를 스레드 개수만큼
    data = (int *)malloc(sizeof(int) * size);
    
    //fill *data for the number of inputs
    for(int i = 0; i < size; i++){
        data[i] = count;
        count++;
    }
    
    //allocated number of threads 
    for(int i = 0; i < noThread; i++){
        threadInfo[i].threadIdx = i;
        threadInfo[i].noThread = noThread;
        threadInfo[i].data = data;
        threadInfo[i].size = size;
    }
    
    //create thread
    for(int t = 0; t < noThread; t++){
        thr_id = pthread_create(&tid[t], NULL, ThreadFn_sum, &threadInfo[t]);
        assert(!thr_id);
    }
    for(int t = 0; t < noThread; t++){
        result = pthread_join(tid[t], NULL);
        total_sum += threadInfo[t].result;
        assert(!result);
    }
    
    printf("Total sum = %lld\n", total_sum);
    printf("Total sum (formula) = %d\n", (size-1)*size/2);
    
    //free()
    free(tid);
    free(threadInfo);
    free(data);
    
    return 0;
}

