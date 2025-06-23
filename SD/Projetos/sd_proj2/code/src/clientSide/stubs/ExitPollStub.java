package clientSide.stubs;

import commInfra.Message;
import commInfra.MessageType;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class ExitPollStub {

    private String serverHost;
    private int serverPort;

    public ExitPollStub(String serverHost, int serverPort) {
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

    public void conductExitPoll(int voterID) {
        sendMessage(MessageType.CONDUCT_EXIT_POLL, voterID);
    }

    public void conductRandomPoll() {
        sendMessage(MessageType.CONDUCT_RANDOM_POLL, null);
    }

    public boolean isActive() {
        Object response = sendMessage(MessageType.IS_ACTIVE, null);
        return response != null && (Boolean) response;
    }    

    public void deactivate() {
        sendMessage(MessageType.DEACTIVATE, null);
    }
    
    public void printStatistics() {
        sendMessage(MessageType.PRINT_STATISTICS, null);
    }    

    public boolean simulationEnded() {
        return (boolean) sendMessage(MessageType.SIMULATION_ENDED, null);
    }    

    private Object sendMessage(MessageType type, Object data) {
        try (Socket socket = new Socket(serverHost, serverPort);
            ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream());
            ObjectInputStream in = new ObjectInputStream(socket.getInputStream())) {

            Message message = new Message(type, data);
            out.writeObject(message);

            Object responseObj = in.readObject();
            if (responseObj instanceof Message) {
                return ((Message) responseObj).getData();
            } else {
                System.err.println("[ERROR] Unexpected response type: " + responseObj);
                return null;
            }

        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
        
    }
}
