package GUI;

import Monitor.PollingStation;
import Repository.LogRepository;
import javax.swing.*;
import javax.swing.table.DefaultTableModel;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class ElectionSimulationGUI extends JFrame {
    private PollingStation pollingStation;
    private LogRepository logRepository;
    private JButton endSimulationButton;
    private DefaultTableModel tableModel;
    private JTable table;
    private int scoreA = 0;
    private int scoreB = 0;

    public ElectionSimulationGUI(LogRepository logRepository) {
        this.logRepository = logRepository;
        setTitle("Election Day Simulation");
        setSize(800, 600);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        endSimulationButton = new JButton("End Simulation");
        endSimulationButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (pollingStation != null) {
                    pollingStation.closeStation();
                }
                updateTable("Closed", "", "", "", String.valueOf(scoreA), String.valueOf(scoreB), "");
                logRepository.saveTableToFile(tableModel, "election_table.csv");
                dispose(); // Fecha a janela
                System.exit(0); // Termina o programa
            }
        });

        JPanel buttonPanel = new JPanel();
        buttonPanel.add(endSimulationButton);
        add(buttonPanel, BorderLayout.SOUTH);

        String[] columnNames = { "Door", "Voter", "Clerk", "Booth", "ScoreA", "ScoreB", "Exit" };
        tableModel = new DefaultTableModel(columnNames, 0);
        table = new JTable(tableModel);
        add(new JScrollPane(table), BorderLayout.CENTER);

        updateTable("Open", "", "", "", "", "", "");
    }

    public void setPollingStation(PollingStation pollingStation) {
        this.pollingStation = pollingStation;
    }

    public void updateTable(String door, String voter, String clerk, String booth, String scoreA, String scoreB,
            String exit) {
        tableModel.addRow(new Object[] { door, voter, clerk, booth, scoreA, scoreB, exit });
    }

    public void updateBooth(String booth) {
        int lastRow = tableModel.getRowCount() - 1;
        tableModel.setValueAt(booth, lastRow, 3); // Atualiza a coluna "Booth" na última linha
    }

    public void incrementScoreA() {
        scoreA++;
        updateTable("", "", "", "", String.valueOf(scoreA), "", "");
    }

    public void incrementScoreB() {
        scoreB++;
        updateTable("", "", "", "", "", String.valueOf(scoreB), "");
    }
}
