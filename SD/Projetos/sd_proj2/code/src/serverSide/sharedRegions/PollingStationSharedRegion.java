package serverSide.sharedRegions;

import commInfra.MemException;
import commInfra.MemFIFO;
import java.util.Random;

public class PollingStationSharedRegion {

    private MemFIFO<Integer> voterQueue; // Fila de eleitores
    private int votersProcessed; // Número de eleitores processados
    private boolean stationOpen; // Estado da estação de votação
    private int stationCapacity; // Capacidade da estação de votação

    public PollingStationSharedRegion(int stationCapacity) { 
        this.voterQueue = new MemFIFO<>(stationCapacity);
        this.votersProcessed = 0;
        this.stationOpen = false;
        this.stationCapacity = stationCapacity;
        System.out.println("Polling station initialized with capacity: " + stationCapacity);
    }

    public synchronized void openStation() {
        stationOpen = true;
        System.out.println("Polling station is now open.");
        notifyAll();
    }

    public synchronized void closeStation() {
        stationOpen = false;
        System.out.println("Polling station is now closed.");
        notifyAll();
    }

    public synchronized void enterQueue(int voterID) throws MemException {
        while (!stationOpen || voterQueue.getSize() >= stationCapacity) {
            try {
                wait();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        voterQueue.write(voterID);
        System.out.println("[VOTER " + voterID + "] Entered the queue.");
        notifyAll();
    }

    public synchronized boolean validateID(int voterID) {
        System.out.println("[VOTER " + voterID + "] Validating ID...");
        try {
            Thread.sleep(new Random().nextInt(6) + 5); // 5-10ms
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return true; // Simulação: todos os IDs são válidos
    }

    public synchronized void castVote(int voterID) {
        System.out.println("[VOTER " + voterID + "] Casting vote...");
        try {
            Thread.sleep(new Random().nextInt(16)); // 0-15ms
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    public synchronized void exitStation(int voterID) {
        System.out.println("[VOTER " + voterID + "] Exited the polling station.");
        votersProcessed++;
        notifyAll();
    }

    public synchronized boolean hasVoters() {
        return !voterQueue.getQueue().isEmpty() || (stationOpen && !simulationEnded());
    }    

    public synchronized void processNextVoter() {
        try {
            while (voterQueue.getQueue().isEmpty() && stationOpen) {
                wait();
            }
            if (!voterQueue.getQueue().isEmpty()) {
                int voterID = voterQueue.read();
                System.out.println("[POLL CLERK] Processing voter " + voterID);
            }
        } catch (MemException | InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    public synchronized boolean simulationEnded() {
        return votersProcessed >= stationCapacity;
    }    
    
}
