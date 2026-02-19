package commInfra;

import java.util.LinkedList;
import java.util.Queue;

public class MemFIFO<R> extends MemObject<R> {

    private Queue<R> queue;

    public MemFIFO(int size) {
        super(size);
        queue = new LinkedList<>();
    }

    public Queue<R> getQueue() {
        return queue;
    }

    public void setQueue(Queue<R> queue) {
        this.queue = queue;
    }

    @Override
    public synchronized void write(R item) throws MemException {
        if (queue.size() >= size) {
            throw new MemException("FIFO is full");
        }
        queue.add(item);
    }

    @Override
    public synchronized R read() throws MemException {
        if (queue.isEmpty()) {
            throw new MemException("FIFO is empty");
        }
        return queue.poll();
    }
}
