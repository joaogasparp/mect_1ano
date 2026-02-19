package Threads;

import Monitor.ExitPoll;
import java.util.List;
import java.util.Random;

public class Pollster implements Runnable {
    private enum State {
        INIT, WAIT, CONDUCT_POLL, DONE
    }

    private State currentState;
    private ExitPoll exitPoll;

    public Pollster(ExitPoll exitPoll) {
        this.exitPoll = exitPoll;
        this.currentState = State.INIT;
    }

    @Override
    public void run() {
        while (currentState != State.DONE) {
            try {
                switch (currentState) {
                    case INIT:
                        System.out.println("[Pollster] INIT");
                        currentState = State.WAIT;
                        break;

                    case WAIT:
                        // Aguarda um intervalo para simular o tempo entre abordagens
                        Thread.sleep(1000);
                        // Verifica se o monitor de exit poll ainda está ativo
                        if (!exitPoll.isActive()) {
                            currentState = State.DONE;
                        } else if (Math.random() < 0.1) { // 80% de chance de conduzir enquete
                            currentState = State.CONDUCT_POLL;
                        } else {
                            currentState = State.WAIT;
                        }
                        break;

                    case CONDUCT_POLL:
                        // Seleciona um ID aleatório para simular a abordagem
                        List<Integer> voters = exitPoll.getEligibleVoters();
                        if (!voters.isEmpty()) {
                            int index = new Random().nextInt(voters.size());
                            int voterID = voters.get(index);
                            String result = exitPoll.conductExitPoll(voterID);
                            System.out.println("[Pollster] " + result);
                        }
                        currentState = State.WAIT;
                        break;

                    case DONE:
                        // Estado de término
                        System.out.println("[Pollster] DONE.");
                        break;
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                currentState = State.DONE;
            }
        }
        System.out.println("[Pollster] Finalizado.");
    }
}
