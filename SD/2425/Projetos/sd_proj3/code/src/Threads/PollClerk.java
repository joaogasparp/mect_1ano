package Threads;

import Monitor.PollingStation;
import Threads.Voter;

public class PollClerk implements Runnable {
    private enum State {
        INIT, WAIT_FOR_VOTER, VALIDATE, DONE
    }

    private State currentState;
    private PollingStation pollingStation;
    private int maxVoters;
    private int processedVoters;
    private Voter currentVoter;

    public PollClerk(PollingStation pollingStation, int maxVoters) {
        this.pollingStation = pollingStation;
        this.maxVoters = maxVoters;
        this.processedVoters = 0;
        this.currentState = State.INIT;
    }

    @Override
    public void run() {
        while (currentState != State.DONE) {
            try {
                switch (currentState) {
                    case INIT:
                        System.out.println("[PollClerk] INIT - Opening station.");
                        pollingStation.openStation();
                        currentState = State.WAIT_FOR_VOTER;
                        break;

                    case WAIT_FOR_VOTER:
                        currentVoter = pollingStation.dequeueVoter();
                        if (currentVoter != null) {
                            System.out.println("[PollClerk] Processing Voter " + currentVoter.getVoterID());
                            currentState = State.VALIDATE;
                        } else {
                            Thread.sleep(50);
                        }
                        break;

                    case VALIDATE:
                        boolean valid = pollingStation.validateID(currentVoter);
                        synchronized (currentVoter) {
                            currentVoter.setValidated(valid);
                        }
                        if (valid) {
                            processedVoters++;
                            System.out.println("[PollClerk] Voter " + currentVoter.getVoterID() + " validated.");
                        } else {
                            System.out.println(
                                    "[PollClerk] Voter " + currentVoter.getVoterID() + " already voted (rejected).");
                        }

                        if (processedVoters >= maxVoters && !pollingStation.hasVotersInside()) {
                            pollingStation.closeStation();
                            currentState = State.DONE;
                        } else {
                            currentState = State.WAIT_FOR_VOTER;
                        }
                        break;
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                break;
            }
        }
        System.out.println("[PollClerk] DONE.");
    }
}
