package serverSide.sharedRegions;

import commInfra.Message;
import commInfra.MessageType;

public class ExitPollInterface {

    private ExitPollSharedRegion exitPoll;

    public ExitPollInterface(ExitPollSharedRegion exitPoll) {
        this.exitPoll = exitPoll;
    }

    public ExitPollSharedRegion getExitPoll() {
        return exitPoll;
    }

    public void setExitPoll(ExitPollSharedRegion exitPoll) {
        this.exitPoll = exitPoll;
    }

    public Message processAndReply(Message request) {
        Message response;
        switch (request.getType()) {
            case CONDUCT_EXIT_POLL:
                exitPoll.conductExitPoll();
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;
            case CONDUCT_RANDOM_POLL:
                exitPoll.conductRandomPoll();
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;
            case IS_ACTIVE:
                boolean isActive = exitPoll.isActive();
                response = new Message(MessageType.RESPONSE, isActive);
                break;
            case DEACTIVATE:
                exitPoll.deactivate();
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;            
            case PRINT_STATISTICS:
                exitPoll.printStatistics();
                response = new Message(MessageType.RESPONSE, Boolean.TRUE);
                break;
            case SIMULATION_ENDED:
                boolean ended = exitPoll.simulationEnded();
                response = new Message(MessageType.RESPONSE, ended);
                break;            
            default:
                response = new Message(MessageType.ERROR, Boolean.FALSE);
        }
        return response;
    }    
}
