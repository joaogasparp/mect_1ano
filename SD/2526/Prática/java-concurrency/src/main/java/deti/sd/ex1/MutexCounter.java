package deti.sd.ex1;

import java.util.concurrent.locks.ReentrantLock;

public class MutexCounter implements Counter {
    private int value = 0;
    private final ReentrantLock lock = new ReentrantLock();

    @Override
    public void increment() {
        lock.lock();
        try {
            value++;
        } finally {
            lock.unlock();
        }
    }

    @Override
    public int getValue() {
        lock.lock();
        try {
            return value;
        } finally {
            lock.unlock();
        }
    }

    @Override
    public boolean incrementIfEven() {
        lock.lock();
        try {
            if (value % 2 == 0) {
                value++;
                return true;
            }
            return false;
        } finally {
            lock.unlock();
        }
    }
}
