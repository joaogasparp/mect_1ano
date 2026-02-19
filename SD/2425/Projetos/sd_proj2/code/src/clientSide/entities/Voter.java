package clientSide.entities;

import clientSide.main.ClientElectionDay;
import clientSide.stubs.ExitPollStub;
import clientSide.stubs.PollingStationStub;
import java.util.List;
import java.util.Random;

public class Voter implements Runnable {

    private int voterID;
    private PollingStationStub pollingStationStub;
    private ExitPollStub exitPollStub;
    private List<Thread> voterThreads;

    public Voter(int voterID, PollingStationStub pollingStationStub, ExitPollStub exitPollStub, List<Thread> voterThreads) {
        this.voterID = voterID;
        this.pollingStationStub = pollingStationStub;
        this.exitPollStub = exitPollStub;
        this.voterThreads = voterThreads;
    }
    public int getVoterID() {
        return voterID;
    }

    public void setVoterID(int voterID) {
        this.voterID = voterID;
    }

    public PollingStationStub getPollingStationStub() {
        return pollingStationStub;
    }

    public void setPollingStationStub(PollingStationStub pollingStationStub) {
        this.pollingStationStub = pollingStationStub;
    }

    public ExitPollStub getExitPollStub() {
        return exitPollStub;
    }

    public void setExitPollStub(ExitPollStub exitPollStub) {
        this.exitPollStub = exitPollStub;
    }

    @Override
    public void run() {
        Thread.currentThread().setName("VOTER-" + voterID);
        try {
            ClientElectionDay.log("[VOTER " + voterID + "] Entering queue...");
            pollingStationStub.enterQueue(voterID);

            ClientElectionDay.log("[VOTER " + voterID + "] Validating ID...");
            boolean validated = pollingStationStub.validateID(voterID);
            if (validated) {
                ClientElectionDay.log("[VOTER " + voterID + "] Casting vote...");
                pollingStationStub.castVote(voterID);

                ClientElectionDay.log("[VOTER " + voterID + "] Conducting exit poll...");
                exitPollStub.conductExitPoll(voterID);
            }

            pollingStationStub.exitStation(voterID);
            ClientElectionDay.log("[VOTER " + voterID + "] Exited polling station.");

            // Reborn logic (30% chance)
            if (new Random().nextInt(100) < 30) {
                int rebornID = voterID + 10000;
                if (!ClientElectionDay.rebornQueue.contains(rebornID)) {
                    ClientElectionDay.rebornQueue.add(rebornID);
                    ClientElectionDay.log("[REBORN REQUEST] Voter " + voterID + " to reborn as " + rebornID);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
