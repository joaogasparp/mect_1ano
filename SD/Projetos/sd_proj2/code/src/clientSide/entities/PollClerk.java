package clientSide.entities;

import clientSide.stubs.PollingStationStub;

public class PollClerk implements Runnable {

    private PollingStationStub pollingStationStub;

    public PollClerk(PollingStationStub pollingStationStub) {
        this.pollingStationStub = pollingStationStub;
    }

    public PollingStationStub getPollingStationStub() {
        return pollingStationStub;
    }

    public void setPollingStationStub(PollingStationStub pollingStationStub) {
        this.pollingStationStub = pollingStationStub;
    }

    @Override
    public void run() {
        pollingStationStub.openStation();

        while (!pollingStationStub.simulationEnded()) {
            if (pollingStationStub.hasVoters()) {
                pollingStationStub.processNextVoter();
            }
        }

        pollingStationStub.closeStation();
    }
}
