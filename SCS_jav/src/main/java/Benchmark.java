import com.sun.jna.Native;
import com.sun.jna.platform.win32.BaseTSD;
import com.sun.jna.platform.win32.WinNT;
import com.sun.jna.win32.StdCallLibrary;

import java.io.*;
import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;
import java.util.concurrent.*;
import java.util.concurrent.locks.*;

public class Benchmark {

    private static final int size = 100000;
    private static final int tests = 100000;
    private static boolean firstOpen = true;
    private static final ReentrantLock lock = new ReentrantLock();
    private static final Condition condition = lock.newCondition();
    private static int turn = 0;

    public static void main(String[] args) throws Exception {
         double val = measureThreadMigration(1000);
          System.out.println(val);
        //benchmark();
    }

    public static double staticMemoryAllocation(int iterations) {
        long finalTime = 0;
        for (int i = 0; i < iterations; i++) {
            long startDynamic = System.nanoTime();
            int[] v = new int[size];
            long endDynamic = System.nanoTime();
            finalTime += endDynamic - startDynamic;
        }
        return (double) finalTime / 1_000_000_000;
    }

    public static double dynamicMemoryAllocation(int iterations, int numberOfElements) {
        long finalTime = 0;
        for (int i = 0; i < iterations; i++) {
            long startDynamic = System.nanoTime();
            int[] dynamicArray = new int[numberOfElements];
            long endDynamic = System.nanoTime();
            finalTime += endDynamic - startDynamic;
        }
        return (double) finalTime / 1_000_000_000;
    }

    public static double staticMemoryAccess() {
        int[] staticArray = new int[size];

        long startStatic = System.nanoTime();
        for (int i = 0; i < tests; i++) {
            for (int j = 0; j < size; j++)
                staticArray[j] = j;
        }
        long endStatic = System.nanoTime();

        return (double) (endStatic - startStatic) / 1_000_000_000;
    }

    public static double dynamicMemoryAccess(int iterations, int numberOfElements) {
        int[] dynamicArray = new int[numberOfElements];

        long startDynamic = System.nanoTime();
        for (int i = 0; i < iterations; i++) {
            for (int j = 0; j < numberOfElements; j++)
                dynamicArray[j] = j;
        }
        long endDynamic = System.nanoTime();

        return (double) (endDynamic - startDynamic) / 1_000_000_000;
    }

    public static double threadCreation(int iterations) {
        long finalTime = 0;
        for (int i = 0; i < iterations; i++) {
            long start = System.nanoTime();
            Thread thread = new Thread(() -> {});
            thread.start();
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            long end = System.nanoTime();
            finalTime += end - start;
        }
        return (double) finalTime / 1_000_000_000;
    }

    public static class ContextSwitchThread implements Runnable {
        private final int id;

        public ContextSwitchThread(int id) {
            this.id = id;
        }

        @Override
        public void run() {
            for (int i = 0; i < 10; i++) {
                lock.lock();
                try {
                    while (turn != id) {
                        condition.await();
                    }

                    turn = 1 - id;
                    condition.signal();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                } finally {
                    lock.unlock();
                }
            }
        }
    }

    public static double measureContextSwitchTime(int iterations) {
        long finalTime = 0;
        for (int i = 0; i < iterations; i++) {
            Thread thread1 = new Thread(new ContextSwitchThread(0));
            Thread thread2 = new Thread(new ContextSwitchThread(1));

            long start = System.nanoTime();
            thread1.start();
            thread2.start();
            try {
                thread1.join();
                thread2.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            long end = System.nanoTime();
            finalTime += end - start;
        }
        return (double) finalTime / 1_000_000_000;
    }



    // Native interface for Kernel32.dll
    public interface Kernel32 extends StdCallLibrary {
        Kernel32 INSTANCE = Native.load("kernel32", Kernel32.class);

        WinNT.HANDLE GetCurrentThread();

        long SetThreadAffinityMask(WinNT.HANDLE hThread, BaseTSD.SIZE_T dwThreadAffinityMask);
    }

    public static double measureThreadMigration(int iterations) {
        WinNT.HANDLE thread = Kernel32.INSTANCE.GetCurrentThread();
        long affinityMask1 = 0x1;
        long affinityMask2 = 0x2;


        double startTime = System.nanoTime();
        for (int i = 0; i < iterations; i++) {
            long result = Kernel32.INSTANCE.SetThreadAffinityMask(thread, new BaseTSD.SIZE_T(affinityMask1));
            if (result == 0) {
                throw new RuntimeException("Failed to set affinity mask to CPU 1");
            }

            result = Kernel32.INSTANCE.SetThreadAffinityMask(thread, new BaseTSD.SIZE_T(affinityMask2));
            if (result == 0) {
                throw new RuntimeException("Failed to set affinity mask to CPU 2");
            }
        }
        double endTime = System.nanoTime();

        return (endTime - startTime) / 1_000_000_000.0;
    }



    public static void writeToFile(String filename, double time, int i) {
        try (FileWriter fileWriter = new FileWriter(filename, !firstOpen)) {
            fileWriter.write(i + " " + time + "\n");
            firstOpen = false;
        } catch (IOException e) {
            System.err.println("Error opening file!\n");
        }
    }

    public static void benchmark() {
        double avgTime;
        int k = 0;
        for (int i = 10000; i <= 1000010000; i += 10000000) {
            System.out.println(k++);
            avgTime = dynamicMemoryAllocation(tests, i);
            writeToFile("mem_allocation_java.txt", avgTime, i);

            avgTime = dynamicMemoryAccess(tests, i);
            writeToFile("dynamic_memory_access_java.txt", avgTime, i);

            avgTime = threadCreation(i/100000);
            writeToFile("thread_creation_java.txt", avgTime, i/100000);

            avgTime = measureContextSwitchTime(i/100000);
            writeToFile("measure_context_switch_time_java.txt", avgTime, i/100000);

            avgTime = measureThreadMigration(i/100000);
            writeToFile("measure_thread_migration_java.txt", avgTime, i/100000);
        }
    }
}
