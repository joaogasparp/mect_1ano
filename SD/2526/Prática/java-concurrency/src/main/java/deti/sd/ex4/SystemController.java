package deti.sd.ex4;

import java.util.concurrent.CountDownLatch;

public class SystemController {
    private static final int NUM_WORKERS = 4;

    public static void main(String[] args) {
        CountDownLatch latch = new CountDownLatch(NUM_WORKERS);

        System.out.println("Controller: Waiting for " + NUM_WORKERS + " workers to initialize...");

        for (int i = 1; i <= NUM_WORKERS; i++) {
            new Thread(new DistributedWorker(i, latch)).start();
        }

        try {
            latch.await();
        } catch (InterruptedException e) {
            System.err.println("Controller: Startup sequence interrupted.");
        }

        System.out.println("Controller: All workers ready. STARTING DISTRIBUTED SYSTEM.");
    }
}
