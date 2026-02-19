package deti.sd.ex1;

import java.util.concurrent.atomic.AtomicInteger;

public class AtomicCounter implements Counter {
    private final AtomicInteger value = new AtomicInteger(0);

    @Override
    public void increment() {
        value.incrementAndGet();
    }

    @Override
    public int getValue() {
        return value.get();
    }

    @Override
    public boolean incrementIfEven() {
        while (true) {
            int current = value.get();
            if (current % 2 != 0) {
                return false;
            }
            if (value.compareAndSet(current, current + 1)) {
                return true;
            }
        }
    }
}
