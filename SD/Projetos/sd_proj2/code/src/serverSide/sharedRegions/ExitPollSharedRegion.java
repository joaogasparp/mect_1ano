package serverSide.sharedRegions;

import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;

public class ExitPollSharedRegion {

    private AtomicBoolean active;
    private int totalVoters; // Total de eleitores que passaram pela pesquisa
    private int sampledVoters; // Número de eleitores abordados na pesquisa
    private int truthfulResponses; // Número de respostas verdadeiras
    private int falseResponses; // Número de respostas falsas

    public ExitPollSharedRegion() {
        this.active = new AtomicBoolean(true);
        this.totalVoters = 0;
        this.sampledVoters = 0;
        this.truthfulResponses = 0;
        this.falseResponses = 0;
    }

    public synchronized void conductExitPoll() {
        System.out.println("Conducting exit poll...");
        totalVoters++;
    }

    public synchronized void conductRandomPoll() {
        totalVoters++;
    
        Random random = new Random();
        if (random.nextInt(100) < 20) {
            sampledVoters++;
            System.out.println("Pollster approached a voter.");
    
            if (random.nextInt(100) < 60) {
                System.out.println("Voter responded to the pollster.");
                if (random.nextInt(100) < 20) {
                    falseResponses++;
                    System.out.println("Voter provided false information.");
                } else {
                    truthfulResponses++;
                    System.out.println("Voter provided truthful information.");
                }
            } else {
                System.out.println("Voter chose not to respond.");
            }
        }
    }    

    public boolean isActive() {
        return active.get();
    }

    public void deactivate() {
        active.set(false);
        System.out.println("Exit poll deactivated.");
    }

    public synchronized void printStatistics() {
        System.out.println("Exit Poll Statistics:");
        System.out.println("Total voters: " + totalVoters);
        System.out.println("Sampled voters: " + sampledVoters);
        System.out.println("Truthful responses: " + truthfulResponses);
        System.out.println("False responses: " + falseResponses);
    }

    public synchronized boolean simulationEnded() {
        return !isActive() || totalVoters >= 10;
    }
    
}
