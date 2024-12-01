#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <windows.h>
#include <processthreadsapi.h>

#define size 100000
#define tests 100000

double static_memory_allocation(int iterations) {
    clock_t final = 0;
    for (int i = 0; i < iterations; i++) {
        clock_t start_dynamic = clock();
        int v[size];
        clock_t end_dynamic = clock();
        final += end_dynamic - start_dynamic;
    }
    return (double)(final) / CLOCKS_PER_SEC;
}

double dynamic_mem_allocation(int iterations, int NumberOfElements) {
    clock_t final = 0;
    for (int i = 0; i < iterations; i++) {
        clock_t start_dynamic = clock();
        int* dynamic_array = (int*) malloc(NumberOfElements * sizeof(int));
        clock_t end_dynamic = clock();
        final += end_dynamic - start_dynamic;
        free(dynamic_array);
    }
    return (double)(final) / CLOCKS_PER_SEC;
}

double static_memory_access() {
    int static_array[size];

    clock_t start_static = clock();
    for (int i = 0; i < tests; i++) {
        for (int j = 0; j < size; j++)
            static_array[j] = j;
    }
    clock_t end_static = clock();

    return (double)(end_static - start_static) / CLOCKS_PER_SEC;
}

double dynamic_memory_access(int iterations, int NrOfElements) {
    int* dynamic_array = (int*) malloc(NrOfElements * sizeof(int));

    clock_t start_dynamic = clock();
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < NrOfElements; j++)
            dynamic_array[j] = j;
    }
    clock_t end_dynamic = clock();

    free(dynamic_array);
    return (double)(end_dynamic - start_dynamic) / CLOCKS_PER_SEC;
}

void* thread_function(void* arg) {
    return nullptr;
}

double thread_creation(int iterations) {
    clock_t final = 0;
    for (int i = 0; i < iterations; i++) {
        pthread_t thread;
        clock_t start = clock();
        pthread_create(&thread, nullptr, thread_function, nullptr);
        pthread_join(thread, nullptr);
        clock_t end = clock();
        final += end - start;
    }
    return (double)(final) / CLOCKS_PER_SEC;
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
    return nullptr;
}

double measure_context_switch_time(int iterations) {
    clock_t final = 0;
    for (int i = 0; i < iterations; i++) {
        pthread_t thread1, thread2;
        int id1 = 0, id2 = 1;

        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);

        pthread_create(&thread1, nullptr, thread_function2, &id1);
        pthread_create(&thread2, nullptr, thread_function2, &id2);
        clock_t start = clock();
        void* time1;
        void* time2;
        pthread_join(thread1, &time1);
        pthread_join(thread2, &time2);

        clock_t end = clock();
        final += end - start;
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
    return (double)(final) / CLOCKS_PER_SEC;
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

void write_to_file(const std::string& filename, double time, int i) {
    std::ofstream file;
    if (first_open) {
        file.open(filename, std::ios::out);
    } else {
        file.open(filename, std::ios::app);
    }
    if (!file) {
        std::cerr << "Error opening file!\n";
        return;
    }
    file << i << " " << time << "\n";
    file.close();
    first_open = false;
}

void benchmark() {
    double avg_time;
    for (int i = 10000; i <= 100000; i += 10000) {
        avg_time = dynamic_mem_allocation(tests, i);
        write_to_file("mem_allocation_cpp.txt", avg_time, i);

        avg_time = dynamic_memory_access(tests, i);
        write_to_file("dynamic_memory_access_cpp.txt", avg_time, i);

        avg_time = thread_creation(i);
        write_to_file("thread_creation_cpp.txt", avg_time, i);

        avg_time = measure_context_switch_time(i);
        write_to_file("measure_context_switch_time_cpp.txt", avg_time, i);

        avg_time = measure_thread_migration(i);
        write_to_file("measure_thread_migration_cpp.txt", avg_time, i);
    }
}

int main() {
    benchmark();
    return 0;
}
