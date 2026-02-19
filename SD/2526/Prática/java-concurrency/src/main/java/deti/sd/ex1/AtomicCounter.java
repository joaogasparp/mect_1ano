package deti.sd.ex1;

import java.util.concurrent.atomic.AtomicInteger;

public class AtomicCounter implements Counter {
    private final AtomicInteger value = new AtomicInteger(0);

    @Override
    public void increment() {
        // TODO: Use AtomicInteger methods to increment
    }

    @Override
    public int getValue() {
        // TODO: Retrieve the atomic value
        return 0;
    }

    @Override
    public boolean incrementIfEven() {
        // TODO: Use compareAndSet in a loop for a lock-free update
        return false;
    }
}
