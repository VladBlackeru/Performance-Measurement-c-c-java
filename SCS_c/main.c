#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <windows.h>
#include <processthreadsapi.h>


#define size 100000
#define tests 100000

void static_mem(){
    int static_array[size];
}
void dynamic_mem(){
    int* dynamic_array = (int*) malloc(size * sizeof(int));
    free(dynamic_array);
}
void allot_memory() {
    // Alocare statică
    clock_t start_static = clock();
    for(int i = 0; i < tests;i++)
        static_mem();
    clock_t end_static = clock();

    // Alocare dinamică
    clock_t start_dynamic = clock();
    for(int i = 0; i < tests;i++)
        dynamic_mem();
    clock_t end_dynamic = clock();

    printf("Timp alocare statica: %lf sec\n", (double)(end_static - start_static)/CLOCKS_PER_SEC);
    printf("Timp alocare dinamica: %lf sec\n", (double)(end_dynamic - start_dynamic)/CLOCKS_PER_SEC);


}


void static_mem_acc(int static_array[]){
    for(int i = 0; i < size; i++)
        static_array[i] = i;
}
void dynamic_mem_acc(int dynamic_array[]){
    for(int i = 0; i < size; i++)
        dynamic_array[i] = i;
}

void mem_access(){

    int* dynamic_array = (int*) malloc(size * sizeof(int));
    int static_array[size];

    clock_t start_static = clock();
    for(int i = 0; i < tests;i++)
        static_mem_acc(static_array);
    clock_t end_static = clock();

    clock_t start_dynamic = clock();
    for(int i = 0; i < tests;i++)
        dynamic_mem_acc(dynamic_array);
    clock_t end_dynamic = clock();

    free(dynamic_array);
    printf("Timp accesare statica: %lf sec\n", (double)(end_static - start_static)/CLOCKS_PER_SEC);
    printf("Timp accesare dinamica: %lf sec\n", (double)(end_dynamic - start_dynamic)/CLOCKS_PER_SEC);
}

void* thread_function(void* arg) {
    return NULL;
}
void create_th(){
    pthread_t thread;
    pthread_create(&thread, NULL, thread_function, NULL);
    pthread_join(thread, NULL);
}

void thread_creation(){
    clock_t start = clock();
    for(int i = 0; i < tests;i++)
        create_th();
    clock_t end = clock();

    printf("Timp creare thread: %lf secunde\n", (double)(end - start)/CLOCKS_PER_SEC);
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

void measure_context_switch_time() {
    clock_t start = clock();
    for (int i = 0; i < tests; i++) {
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


        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
    clock_t end = clock();
    printf("Timp context switch: %lf secunde\n",(double)(end - start)/CLOCKS_PER_SEC);
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

    return total_time / iterations;
}



int main(void) {
    double avg_time = measure_thread_migration(50000);
    printf("Average thread migration time: %f seconds\n", avg_time);
    //allot_memory();
    //mem_access();
    //thread_creation();
    //measure_context_switch_time();
    return 0;
}
