package deti.sd.ex2;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Gateway {
    private static final int THREAD_POOL_SIZE = 5;
    private static final int TOTAL_TASKS = 50;

    public static void main(String[] args) {
        ExecutorService executor = Executors.newFixedThreadPool(THREAD_POOL_SIZE);

        System.out.println("Gateway: Dispatching " + TOTAL_TASKS + " tasks...");

        for (int i = 0; i < TOTAL_TASKS; i++) {
            executor.submit(new HeavyTask(i + 1));
        }

        executor.shutdown();

        try {
            executor.awaitTermination(1, TimeUnit.MINUTES);
        } catch (InterruptedException e) {
            System.err.println("Gateway shutdown interrupted.");
        }

        System.out.println("Gateway: All tasks finished. Shutting down.");
    }
}
