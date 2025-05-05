package Threads;

import Monitor.PollingStation;
import Monitor.ExitPoll;

public class Voter implements Runnable {
    private enum State {
        INIT, ENTER_QUEUE, WAIT_FOR_VALIDATION, VOTE, EXIT, REBORN, DONE
    }

    private State currentState;
    private int voterID;
    private PollingStation pollingStation;
    private ExitPoll exitPoll;
    private boolean validated = false; // Flag que indica se o eleitor foi validado

    public Voter(int voterID, PollingStation pollingStation, ExitPoll exitPoll) {
        this.voterID = voterID;
        this.pollingStation = pollingStation;
        this.exitPoll = exitPoll;
        this.currentState = State.INIT;
    }

    // Método chamado pelo PollClerk para informar que o Voter foi validado
    public synchronized void setValidated(boolean valid) {
        this.validated = valid;
        notify(); // Libera o Voter para continuar o processo
    }

    // Aguarda a validação antes de seguir
    public synchronized void waitForValidation() throws InterruptedException {
        while (!validated) {
            wait();
        }
    }

    @Override
    public void run() {
        while (currentState != State.DONE) {
            try {
                switch (currentState) {
                    case INIT:
                        System.out.println("[Voter " + voterID + "] INIT");
                        currentState = State.ENTER_QUEUE;
                        break;

                    case ENTER_QUEUE:
                        pollingStation.enterQueue(this);
                        System.out.println("[Voter " + voterID + "] Entered queue");
                        currentState = State.WAIT_FOR_VALIDATION;
                        break;

                    case WAIT_FOR_VALIDATION:
                        waitForValidation(); // Aguarda o PollClerk validar o ID
                        if (!validated) {
                            System.out.println("[Voter " + voterID + "] Validation failed; exiting");
                            currentState = State.DONE;
                        } else {
                            currentState = State.VOTE;
                        }
                        break;

                    case VOTE:
                        pollingStation.castVote(this);
                        currentState = State.EXIT;
                        break;

                    case EXIT:
                        pollingStation.processExit(this);
                        String result = exitPoll.conductExitPoll(voterID);
                        System.out.println("[Voter " + voterID + "] " + result);
                        currentState = State.REBORN;
                        break;

                    case REBORN:
                        if (shouldReborn() && pollingStation.isOpen()) {
                            int newID;
                            do {
                                newID = (int) (Math.random() * 1000);
                            } while (pollingStation.hasVoted(newID)); // Evita IDs repetidos

                            System.out.println("[Voter " + voterID + "] Reborn as Voter " + newID);
                            Voter rebornVoter = new Voter(newID, pollingStation, exitPoll);
                            new Thread(rebornVoter).start();
                        }
                        currentState = State.DONE;
                        break;
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                break;
            }
        }
        System.out.println("[Voter " + voterID + "] DONE.");
    }

    private boolean shouldReborn() {
        return Math.random() < 0.3; // 30% de chance para renascer
    }

    public int getVoterID() {
        return voterID;
    }
}
