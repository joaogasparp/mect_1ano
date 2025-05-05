package Monitor;

import Repository.LogRepository;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

public class ExitPoll {
    private LogRepository logRepository;
    private Random random;
    private volatile boolean active; // Flag para controlar a atividade do monitor
    private List<Integer> eligibleVoters = Collections.synchronizedList(new ArrayList<>());

    public ExitPoll(LogRepository logRepository) {
        this.logRepository = logRepository;
        this.random = new Random();
        this.active = true; // Inicialmente ativo
    }

    public synchronized String conductExitPoll(int voterID) throws InterruptedException {
        Thread.sleep(5 + random.nextInt(6));
        boolean willRespond = random.nextDouble() < 0.6; // 60% de chance de responder
        if (willRespond) {
            boolean truthful = random.nextDouble() < 0.8; // 80% de chance de ser verdadeiro
            String response = truthful ? "truthful" : "false";
            eligibleVoters.remove(Integer.valueOf(voterID)); // Remove o eleitor da lista de elegíveis
            return "Voter " + voterID + " responded and was " + response;
        }
        eligibleVoters.remove(Integer.valueOf(voterID)); // Remove o eleitor da lista de elegíveis
        return "Voter " + voterID + " did not respond";
    }

    // Retorna se o monitor ainda está ativo
    public boolean isActive() {
        return active;
    }

    // Permite fechar o monitor de exit poll
    public synchronized void close() {
        active = false;
    }

    public void registerVoter(int voterID) {
        eligibleVoters.add(voterID);
    }

    public List<Integer> getEligibleVoters() {
        return eligibleVoters;
    }
}
