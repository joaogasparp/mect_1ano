package deti.sd.ex3;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public class DistributedBufferSystem {
    public static void main(String[] args) {
        BlockingQueue<DataPacket> buffer = new LinkedBlockingQueue<>(5);

        Thread producerThread = new Thread(new NetworkProducer(buffer));
        Thread consumerThread = new Thread(new DataProcessor(buffer));

        producerThread.start();
        consumerThread.start();
    }
}
