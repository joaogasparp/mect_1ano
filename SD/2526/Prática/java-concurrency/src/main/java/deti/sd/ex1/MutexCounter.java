package deti.sd.ex1;

import java.util.concurrent.locks.ReentrantLock;

public class MutexCounter implements Counter {
    private int value = 0;
    private final ReentrantLock lock = new ReentrantLock();

    @Override
    public void increment() {
        // TODO: Implement using the ReentrantLock
    }

    @Override
    public int getValue() {
        // TODO: Implement thread-safe read
        return 0;
    }

    @Override
    public boolean incrementIfEven() {
        // TODO: Implement conditional update using the lock
        return false;
    }
}
