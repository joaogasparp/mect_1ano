package serverSide.entities;

import commInfra.Message;
import serverSide.sharedRegions.PollingStationInterface;

public class PollingStationService extends Thread {

    private PollingStationInterface pollingStationInterface;
    private Message request;
    private Message response;

    public PollingStationService(PollingStationInterface pollingStationInterface, Message request) {
        this.pollingStationInterface = pollingStationInterface;
        this.request = request;
    }

    public PollingStationInterface getPollingStationInterface() {
        return pollingStationInterface;
    }

    public void setPollingStationInterface(PollingStationInterface pollingStationInterface) {
        this.pollingStationInterface = pollingStationInterface;
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
        response = pollingStationInterface.processAndReply(request);
    }
}
