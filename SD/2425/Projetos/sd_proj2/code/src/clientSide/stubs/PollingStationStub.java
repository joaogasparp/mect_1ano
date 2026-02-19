package clientSide.stubs;

import commInfra.Message;
import commInfra.MessageType;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class PollingStationStub {

    private String serverHost;
    private int serverPort;

    public PollingStationStub(String serverHost, int serverPort) {
        this.serverHost = serverHost;
        this.serverPort = serverPort;
    }

    public String getServerHost() {
        return serverHost;
    }

    public void setServerHost(String serverHost) {
        this.serverHost = serverHost;
    }

    public int getServerPort() {
        return serverPort;
    }

    public void setServerPort(int serverPort) {
        this.serverPort = serverPort;
    }

    public void enterQueue(int voterID) {
        sendMessage(MessageType.ENTER_QUEUE, voterID);

    }

    public boolean validateID(int voterID) {
        return (boolean) sendMessage(MessageType.VALIDATE_ID, voterID);
    }

    public void castVote(int voterID) {
        sendMessage(MessageType.CAST_VOTE, voterID);
    }

    public void exitStation(int voterID) {
        sendMessage(MessageType.EXIT_STATION, voterID);
    }

    public void openStation() {
        sendMessage(MessageType.OPEN_STATION, null);
    }

    public void closeStation() {
        sendMessage(MessageType.CLOSE_STATION, null);
    }

    public boolean hasVoters() {
        return (boolean) sendMessage(MessageType.HAS_VOTERS, null);
    }

    public void processNextVoter() {
        sendMessage(MessageType.PROCESS_NEXT_VOTER, null);
    }

    public boolean simulationEnded() {
        return (boolean) sendMessage(MessageType.SIMULATION_ENDED, null);
    }    

    private Object sendMessage(MessageType type, Object data) {
        try (
            Socket socket = new Socket(serverHost, serverPort);
            ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream());
            ObjectInputStream in = new ObjectInputStream(socket.getInputStream())
        ) {
            Message message = new Message(type, data);
            out.writeObject(message);

            Message response = (Message) in.readObject();
            return response.getData();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
