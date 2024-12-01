#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <windows.h>
#include <processthreadsapi.h>
#include <stdbool.h>


#define size 100000
#define tests 100000


double static_memory_allocation(int iterations){
    clock_t final = 0;
    for(int i = 0; i < iterations;i++){
        clock_t start_dynamic = clock();
        int v[size];
        clock_t end_dynamic = clock();
        final += end_dynamic - start_dynamic;
    }
    return (double)(final)/CLOCKS_PER_SEC;
}

double dynamic_mem_allocation(int iterations, int  NumberOfElements){
    clock_t final = 0;
    for(int i = 0; i < iterations;i++){
        clock_t start_dynamic = clock();
        int* dynamic_array = (int*) malloc(NumberOfElements * sizeof(int));
        clock_t end_dynamic = clock();
        final += end_dynamic - start_dynamic;
        free(dynamic_array);
    }
    return (double)(final)/CLOCKS_PER_SEC;
}

double static_memory_access() {
    int static_array[size];

    clock_t start_static = clock();
    for(int i = 0; i < tests; i++){
        for(int j = 0; j < size; j++)
            static_array[j] = j;
    }
    clock_t end_static = clock();

    return (double)(end_static - start_static) / CLOCKS_PER_SEC;
}

double dynamic_memory_access(int iterations,int NrOfElements) {
    int* dynamic_array = (int*) malloc(NrOfElements * sizeof(int));

    clock_t start_dynamic = clock();
    for(int j = 0; j < NrOfElements; j++)
        dynamic_array[j] = j;
    clock_t end_dynamic = clock();

    free(dynamic_array);
    return (double)(end_dynamic - start_dynamic) / CLOCKS_PER_SEC;
}


void* thread_function(void* arg) {
    return NULL;
}

double thread_creation(int iterations){
    clock_t final = 0;
    for(int i = 0; i < iterations;i++){
        pthread_t thread;
        clock_t start = clock();
        pthread_create(&thread, NULL, thread_function, NULL);
        pthread_join(thread, NULL);
        clock_t end = clock();
        final += end - start;
    }
    return (double)(final)/CLOCKS_PER_SEC;
}



pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0;

void* thread_function2(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);

        while (turn != id) {
            pthread_cond_wait(&cond, &mutex);
        }

        turn = 1 - id; // Switch turn to the other thread
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

double measure_context_switch_time(int iterations) {
    clock_t final = 0;
    for (int i = 0; i < iterations; i++) {
        clock_t start = clock();
        pthread_t thread1, thread2;
        int id1 = 0, id2 = 1;

        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);

        pthread_create(&thread1, NULL, thread_function2, &id1);
        pthread_create(&thread2, NULL, thread_function2, &id2);
        void* time1;
        void* time2;
        pthread_join(thread1, &time1);
        pthread_join(thread2, &time2);

        clock_t end = clock();
        final += (end - start);
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
    return (double)(final)/CLOCKS_PER_SEC;
}


double measure_thread_migration(int iterations) {
    HANDLE thread = GetCurrentThread();
    DWORD_PTR affinityMask1 = 1;
    DWORD_PTR affinityMask2 = 2;
    LARGE_INTEGER start, end, freq;

    QueryPerformanceFrequency(&freq);
    double total_time = 0;

    for (int i = 0; i < iterations; i++) {
        QueryPerformanceCounter(&start);
        SetThreadAffinityMask(thread, affinityMask1);
        SetThreadAffinityMask(thread, affinityMask2);
        QueryPerformanceCounter(&end);
        total_time += (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    }

    return total_time;
}


bool first_open = true;

void write_to_file(const char* filename, double time, int i) {
    FILE *file = NULL;
    if(first_open == true)
        file = fopen(filename, "w");
    else
        file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }
    fprintf(file, "%d %f\n", i, time);
    fclose(file);
}

void benchmark(){
    double avg_time;
    int k = 0;
    for(int i = 10000; i <= 1000010000; i+= 10000000){
        printf("%d \n", k++);
//        avg_time = dynamic_mem_allocation(tests,i);
//        write_to_file("mem_allocation_c.txt", avg_time,i);

//        avg_time = dynamic_memory_access(tests, i);
//        write_to_file("dynamic_memory_access_c.txt", avg_time,i);

        avg_time = thread_creation(i/100000);
        write_to_file("thread_creation_c.txt", avg_time, i/10000);

        avg_time = measure_context_switch_time(i/100000);
        write_to_file("measure_context_switch_time_c.txt", avg_time, i/10000);

        avg_time = measure_thread_migration(i/100000);
        write_to_file("measure_thread_migration_c.txt", avg_time, i/10000);

        first_open = false;
    }


}


int main(void) {
    benchmark();
    return 0;
}
