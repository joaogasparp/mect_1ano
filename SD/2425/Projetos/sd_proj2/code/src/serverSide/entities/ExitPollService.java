package serverSide.entities;

import commInfra.Message;
import serverSide.sharedRegions.ExitPollInterface;

public class ExitPollService extends Thread {

    private ExitPollInterface exitPollInterface;
    private Message request;
    private Message response;

    public ExitPollService(ExitPollInterface exitPollInterface, Message request) {
        this.exitPollInterface = exitPollInterface;
        this.request = request;
    }

    public ExitPollInterface getExitPollInterface() {
        return exitPollInterface;
    }

    public void setExitPollInterface(ExitPollInterface exitPollInterface) {
        this.exitPollInterface = exitPollInterface;
    }

    public Message getRequest() {
        return request;
    }

    public void setRequest(Message request) {
        this.request = request;
    }

    public Message getResponse() {
        return response;
    }

    @Override
    public void run() {
        response = exitPollInterface.processAndReply(request);
    }
}
