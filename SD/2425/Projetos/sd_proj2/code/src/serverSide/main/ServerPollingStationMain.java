package serverSide.main;

import commInfra.Message;
import commInfra.ServerCom;
import serverSide.sharedRegions.PollingStationInterface;
import serverSide.sharedRegions.PollingStationSharedRegion;

public class ServerPollingStationMain {

    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.println("Usage: java serverSide.main.ServerPollingStationMain <stationCapacity: 2-5>");
            System.exit(1);
        }

        int stationCapacity = Integer.parseInt(args[0]);
        if (stationCapacity < 2 || stationCapacity > 5) {
            System.out.println("Station capacity must be between 2 and 5.");
            System.exit(1);
        }

        int port = 22000;
        ServerCom serverCom = null;

        PollingStationSharedRegion pollingStation = new PollingStationSharedRegion(stationCapacity);
        PollingStationInterface pollingStationInterface = new PollingStationInterface(pollingStation);

        try {
            serverCom = new ServerCom(port);
            System.out.println("Polling Station Server is running on port " + port + " with capacity " + stationCapacity);
            while (true) {
                ServerCom clientCom = serverCom.accept();
                new Thread(() -> {
                    try {
                        Object request = clientCom.readObject();
                        Object response = pollingStationInterface.processAndReply((Message) request);
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
