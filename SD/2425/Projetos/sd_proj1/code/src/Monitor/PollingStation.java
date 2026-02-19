package Monitor;

import Repository.LogRepository;
import Repository.VoteCounter;
import GUI.ElectionSimulationGUI;
import Threads.Voter;
import java.util.concurrent.*;
import java.util.*;

public class PollingStation {
    private final int capacity;
    private final Queue<Voter> waitingQueue;
    private final Semaphore stationSemaphore;
    private final Semaphore votingSemaphore;
    private volatile boolean open;
    private ElectionSimulationGUI gui;
    private LogRepository logRepository;
    private VoteCounter voteCounter;
    private Set<Integer> votedIDs;

    public PollingStation(int capacity, ElectionSimulationGUI gui, LogRepository logRepository,
            VoteCounter voteCounter) {
        this.capacity = capacity;
        this.gui = gui;
        this.logRepository = logRepository;
        this.voteCounter = voteCounter;
        waitingQueue = new LinkedList<>();
        stationSemaphore = new Semaphore(capacity);
        votingSemaphore = new Semaphore(1);
        open = false; // Inicialmente fechada
        votedIDs = new HashSet<>();
    }

    public synchronized void openStation() {
        open = true;
        notifyAll(); // Notifica as threads que aguardam que a estação abra
        gui.updateTable("Open", "", "", "", "", "", "");
    }

    public synchronized boolean validateID(Voter v) throws InterruptedException {
        Thread.sleep(5 + (int) (Math.random() * 6));
        if (votedIDs.contains(v.getVoterID())) {
            return false;
        } else {
            votedIDs.add(v.getVoterID());
            gui.updateTable("", "", "Voter " + v.getVoterID(), "", "", "", "");
            return true;
        }
    }

    public void enterQueue(Voter v) throws InterruptedException {
        synchronized (this) {
            while (!open) {
                wait(); // Aguarda até que a estação esteja aberta
            }
        }
        stationSemaphore.acquire();
        synchronized (this) {
            waitingQueue.offer(v);
            gui.updateTable("", "Voter " + v.getVoterID(), "", "", "", "", "");
        }
    }

    public synchronized Voter dequeueVoter() {
        return waitingQueue.poll();
    }

    public void castVote(Voter v) throws InterruptedException {
        votingSemaphore.acquire();
        Thread.sleep((int) (Math.random() * 16));
        String candidate = Math.random() < 0.6 ? "A" : "B";
        voteCounter.addVote(candidate);
        if (candidate.equals("A")) {
            gui.incrementScoreA();
        } else {
            gui.incrementScoreB();
        }
        gui.updateBooth(v.getVoterID() + " voted for " + candidate);
        votingSemaphore.release();
    }

    public void processExit(Voter v) throws InterruptedException {
        Thread.sleep(5 + (int) (Math.random() * 6));
        gui.updateTable("", "", "", "", "", "", "Voter " + v.getVoterID());
        stationSemaphore.release();
    }

    public synchronized boolean hasVotersInside() {
        return !waitingQueue.isEmpty() || (capacity - stationSemaphore.availablePermits()) > 0;
    }

    public synchronized void closeStation() {
        if (!open)
            return; // Se já estiver fechada, não faz nada
        open = false;
        notifyAll();
        gui.updateTable("Closed", "", "", "", "", "", "");
    }

    public synchronized boolean isOpen() {
        return open;
    }

    public synchronized boolean hasVoted(int voterID) {
        return votedIDs.contains(voterID);
    }
}
