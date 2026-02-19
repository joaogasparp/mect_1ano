package Main;

import GUI.ElectionSimulationGUI;
import Monitor.ExitPoll;
import Monitor.PollingStation;
import Repository.LogRepository;
import Repository.VoteCounter;
import Threads.PollClerk;
import Threads.Pollster;
import Threads.Voter;

public class Main extends javax.swing.JFrame {

    private static PollingStation pollingStation;
    private static ElectionSimulationGUI gui;

    public Main() {
        initComponents();
    }

    @SuppressWarnings("unchecked")
    private void initComponents() {
        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        pack();
    }

    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(() -> new Main().setVisible(true));

        if (args.length < 2) {
            System.out.println("Usage: java Main <numberOfVoters> <stationCapacity>\n"
                    + "Number of voters must be between 3 and 10.\n"
                    + "Station capacity must be between 2 and 5.");
            System.exit(1);
        }

        int numberOfVoters = Integer.parseInt(args[0]);
        int stationCapacity = Integer.parseInt(args[1]);

        if (numberOfVoters < 3 || numberOfVoters > 10) {
            System.out.println("Number of voters must be between 3 and 10.");
            System.exit(1);
        }

        if (stationCapacity < 2 || stationCapacity > 5) {
            System.out.println("Station capacity must be between 2 and 5.");
            System.exit(1);
        }

        LogRepository logRepository = new LogRepository();
        VoteCounter voteCounter = new VoteCounter();
        gui = new ElectionSimulationGUI(logRepository);
        pollingStation = new PollingStation(stationCapacity, gui, logRepository, voteCounter);
        ExitPoll exitPoll = new ExitPoll(logRepository);
        gui.setPollingStation(pollingStation);
        gui.setVisible(true);

        // Inicia as threads de PollClerk e Pollster
        Thread pollClerkThread = new Thread(new PollClerk(pollingStation, numberOfVoters));
        Thread pollsterThread = new Thread(new Pollster(exitPoll));
        pollClerkThread.start();
        pollsterThread.start();

        // Cria e inicia as threads dos Voters
        Thread[] voters = new Thread[numberOfVoters];
        for (int i = 0; i < numberOfVoters; i++) {
            voters[i] = new Thread(new Voter(i + 1, pollingStation, exitPoll));
            voters[i].start();
        }

        // Aguarda 5 segundos para a simulação ocorrer
        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        // Fecha a estação e também o monitor de exit poll
        pollingStation.closeStation();
        exitPoll.close();

        // Aguarda todas as threads terminarem
        try {
            for (Thread voter : voters) {
                voter.join();
            }
            pollClerkThread.join();
            pollsterThread.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        System.out.println("End of Simulation");
        System.exit(0);
    }
}
