#include <bits/stdc++.h>
#include <ctime>
#include <thread>
#include <windows.h>
using namespace std;
#define size 100000
#define tests 100000

void static_mem(){
    int static_array[size];
}

void dynamic_mem(){
    int* dynamic_array = new int[size];
    delete[] dynamic_array;
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

    cout << "Timp alocare statica: " << (double)(end_static - start_static)/CLOCKS_PER_SEC << " clocks\n";
    cout << "Timp alocare dinamica: " << (double)(end_dynamic - start_dynamic)/CLOCKS_PER_SEC<< " clocks\n";
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
    int static_array[size];
    int* dynamic_array = new int[size];
    clock_t start_static = clock();
    for(int i = 0; i < tests;i++)
        static_mem_acc(static_array);
    clock_t end_static = clock();

    clock_t start_dynamic = clock();
    for(int i = 0; i < tests;i++)
        dynamic_mem_acc(dynamic_array);
    clock_t end_dynamic = clock();

    cout << "Timp accesare statica: " << (double)(end_static - start_static)/CLOCKS_PER_SEC << " sec\n";
    cout << "Timp accesare dinamica: " << (double)(end_dynamic - start_dynamic)/CLOCKS_PER_SEC<< " sec\n";
    delete[] dynamic_array;
}

void thread_function() {}

void create_th(){
    std::thread t(thread_function);
    t.join();
}

void thread_creation(){
    clock_t start = clock();
    for(int i = 0; i < tests;i++)
        create_th();
    clock_t end = clock();

    std::cout << "Timp creare thread: " << (double)(end - start) /CLOCKS_PER_SEC<< " sec\n";
}



double measure_thread_migration(int iterations) {
    HANDLE thread = GetCurrentThread(); // Get the current thread handle
    DWORD_PTR affinityMask1 = 1;       // CPU 0
    DWORD_PTR affinityMask2 = 2;       // CPU 1
    double total_time = 0;

    for (int i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        SetThreadAffinityMask(thread, affinityMask1); // Migrate to CPU 0
        SetThreadAffinityMask(thread, affinityMask2); // Migrate to CPU 1
        auto end = std::chrono::high_resolution_clock::now();
        total_time += std::chrono::duration<double>(end - start).count();
    }

    return total_time / iterations; // Return the average time
}




int main() {
    double avg_time = measure_thread_migration(100000);
    std::cout << "Average thread migration time: " << avg_time << " seconds" << std::endl;

    //allot_memory();
    //mem_access();
    //thread_creation();
    return 0;
}
