package clientSide.entities;

import clientSide.stubs.ExitPollStub;

public class Pollster implements Runnable {

    private ExitPollStub exitPollStub;

    public Pollster(ExitPollStub exitPollStub) {
        this.exitPollStub = exitPollStub;
    }

    public ExitPollStub getExitPollStub() {
        return exitPollStub;
    }

    public void setExitPollStub(ExitPollStub exitPollStub) {
        this.exitPollStub = exitPollStub;
    }

    @Override
    public void run() {
        while (!exitPollStub.simulationEnded()) {
            if (exitPollStub.isActive()) {
                exitPollStub.conductRandomPoll();
            }
        }
    }
}
