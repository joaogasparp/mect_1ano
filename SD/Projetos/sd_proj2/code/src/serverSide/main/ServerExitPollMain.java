package serverSide.main;

import commInfra.Message;
import commInfra.MessageType;
import commInfra.ServerCom;
import serverSide.sharedRegions.ExitPollInterface;
import serverSide.sharedRegions.ExitPollSharedRegion;

public class ServerExitPollMain {

    public static void main(String[] args) {
        int port = 22001;
        ServerCom serverCom = null;

        ExitPollSharedRegion exitPoll = new ExitPollSharedRegion();
        ExitPollInterface exitPollInterface = new ExitPollInterface(exitPoll);

        try {
            serverCom = new ServerCom(port);
            System.out.println("Exit Poll Server is running...");
            while (true) {
                ServerCom clientCom = serverCom.accept();
                new Thread(() -> {
                    try {
                        Object requestObj = clientCom.readObject();

                        if (!(requestObj instanceof Message)) {
                            System.err.println("[ERROR] Invalid request object: " + requestObj);
                            clientCom.writeObject(new Message(MessageType.ERROR, "Invalid message format"));
                            return;
                        }

                        Message request = (Message) requestObj;
                        Message response = exitPollInterface.processAndReply(request);
                        clientCom.writeObject(response);

                    } catch (Exception e) {
                        e.printStackTrace();
                    } finally {
                        try {
                            clientCom.close();
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }).start();
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (serverCom != null) {
                try {
                    serverCom.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
