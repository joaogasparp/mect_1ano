package serverSide.sharedRegions;

import commInfra.Message;
import commInfra.MessageType;

public class PollingStationInterface {

    private PollingStationSharedRegion pollingStation;

    public PollingStationInterface(PollingStationSharedRegion pollingStation) {
        this.pollingStation = pollingStation;
    }

    public PollingStationSharedRegion getPollingStation() {
        return pollingStation;
    }

    public void setPollingStation(PollingStationSharedRegion pollingStation) {
        this.pollingStation = pollingStation;
    }

    public Message processAndReply(Message request) {
        Message response;
        switch (request.getType()) {
            case ENTER_QUEUE:
                try {
                    pollingStation.enterQueue((int) request.getData());
                    response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                } catch (Exception e) {
                    response = new Message(MessageType.RESPONSE, Boolean.FALSE);
                }
                break;
    
            case VALIDATE_ID:
                boolean isValid = pollingStation.validateID((int) request.getData());
                response = new Message(MessageType.RESPONSE, isValid);
                break;
    
            case CAST_VOTE:
                pollingStation.castVote((int) request.getData());
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;
    
            case EXIT_STATION:
                pollingStation.exitStation((int) request.getData());
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;
    
            case HAS_VOTERS:
                boolean hasVoters = pollingStation.hasVoters();
                response = new Message(MessageType.RESPONSE, hasVoters);
                break;
    
            case PROCESS_NEXT_VOTER:
                pollingStation.processNextVoter();
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;
    
            case OPEN_STATION:
                pollingStation.openStation();
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;
    
            case CLOSE_STATION:
                pollingStation.closeStation();
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;
            
            case SIMULATION_ENDED:
                boolean ended = pollingStation.simulationEnded();
                response = new Message(MessageType.RESPONSE, ended);
                break;                     

            default:
                response = new Message(MessageType.ERROR, Boolean.FALSE);
        }
        return response;
    }
}
