package org.example;

public class Main {
    static int size = 100000;
    static int tests = 100000;
    public static void main(String[] args) {
        allot();
        acces_mem();
        thread_creation();
    }

    static void allot(){

        long startDynamic = System.nanoTime();
        for(int i = 0; i < tests; i++) {
            //int[] dynamicArray = new int[size];
            allot_mem();
        }
        long endDynamic = System.nanoTime();
        System.out.println("Timp alocare memorie dinamica: " + (endDynamic - startDynamic) / 1e9 + " secunde");
    }
    static void allot_mem(){
        int[] dynamicArray = new int[size];
    }
    static void acces_mem(){

        int[] dynamicArray = new int[size];
        long startDynamic = System.nanoTime();
        for(int i = 0; i < tests; i++) {
            acc_mem(dynamicArray, i);
        }
        long endDynamic = System.nanoTime();
        System.out.println("Timp accesare memorie dinamica: " + (endDynamic - startDynamic) / 1e9 + " secunde");
    }
    static void acc_mem(int[] dynamicArray, int iteration){
        for(int i = 0; i < size;i++) {
            dynamicArray[i] = i+iteration;
        }
    }

    static void thread_creation(){
        long start = System.nanoTime();
        for(int i = 0; i < tests; i++) {
            create_thread();
        }
        long end = System.nanoTime();
        System.out.println("Timp creare thread: " + (end - start) / 1e9 + " secunde");
    }

     static void create_thread(){
         Thread t = new Thread(() -> {});
         t.start();
         try {
             t.join();
         } catch (InterruptedException e) {
             e.printStackTrace();
         }
    }
}