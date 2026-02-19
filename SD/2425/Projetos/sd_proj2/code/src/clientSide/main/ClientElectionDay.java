package clientSide.main;

import clientSide.entities.PollClerk;
import clientSide.entities.Pollster;
import clientSide.entities.Voter;
import clientSide.stubs.ExitPollStub;
import clientSide.stubs.PollingStationStub;
import java.io.PrintWriter;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ClientElectionDay {

    // Lista partilhada para guardar todas as threads de votantes (incluindo renascidos)
    public static final List<Thread> voterThreads = Collections.synchronizedList(new LinkedList<>());

    public static final Queue<Integer> rebornQueue = new ConcurrentLinkedQueue<>();

    public static PrintWriter logWriter;

    public static int TOTAL_VOTERS;

    public static void log(String msg) {
        System.out.println(msg);
        if (logWriter != null) {
            logWriter.println(msg);
            logWriter.flush(); // para garantir que é escrito de imediato
        }
    }    
    
    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.println("Usage: java ClientElectionDay <numVoters: 3-10>");
            System.exit(1);
        }
        
        int numVoters = Integer.parseInt(args[0]);

        if (numVoters < 3 || numVoters > 10) {
            System.out.println("Número de votantes deve ser entre 3 e 10.");
            System.exit(1);
        }
        
        TOTAL_VOTERS = numVoters;

        String serverHost = "localhost";
        int basePort = 22000;

        try {
            logWriter = new PrintWriter("log.txt", "UTF-8");
        } catch (Exception e) {
            System.err.println("Erro ao criar log.txt: " + e.getMessage());
            System.exit(1);
        }
        
        // Stubs para comunicação com os servidores
        PollingStationStub pollingStationStub = new PollingStationStub("localhost", 22001);
        ExitPollStub exitPollStub = new ExitPollStub("localhost", 22001);

        // Inicializa as entidades
        PollClerk pollClerk = new PollClerk(pollingStationStub);
        Pollster pollster = new Pollster(exitPollStub);

        // Inicializa os votantes
        for (int i = 0; i < numVoters; i++) {
            Thread voterThread = new Thread(new Voter(i + 1, pollingStationStub, exitPollStub, voterThreads));
            voterThreads.add(voterThread);
            voterThread.start();
        }        

        // Inicia o PollClerk e o Pollster
        Thread pollClerkThread = new Thread(pollClerk);
        Thread pollsterThread = new Thread(pollster);
        pollClerkThread.start();
        pollsterThread.start();

        // Aguarda todos os votantes (incluindo renascidos)
        while (true) {
            List<Thread> snapshot;
            synchronized (voterThreads) {
                snapshot = new LinkedList<>(voterThreads);
            }

            boolean allDone = true;
            for (Thread t : snapshot) {
                try {
                    t.join();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
                if (t.isAlive()) {
                    allDone = false;
                }
            }
            
            boolean rebornsPending = true;

            while (rebornsPending) {
                rebornsPending = false;

                while (!rebornQueue.isEmpty()) {
                    int id = rebornQueue.poll();
                    
                    // Verifica se já existe uma thread com este ID para evitar duplicações
                    boolean alreadyExists = false;
                    synchronized (voterThreads) {
                        for (Thread vt : voterThreads) {
                            if (vt.getName().equals("VOTER-" + id)) {
                                alreadyExists = true;
                                break;
                            }
                        }
                    }
                
                    if (!alreadyExists) {
                        Thread t = new Thread(new Voter(id, pollingStationStub, exitPollStub, voterThreads));
                        t.setName("VOTER-" + id); // Nomeia a thread para facilitar o controlo
                        voterThreads.add(t);
                        t.start();
                    }
                }

                // Espera novos reborn terminarem
                for (Thread t : voterThreads) {
                    try {
                        t.join();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
            }

            if (allDone) break;

            try {
                Thread.sleep(100); // pequena pausa antes de verificar novamente
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        // Espera que o PollClerk termine
        try {
            pollClerkThread.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        // Estatísticas finais
        exitPollStub.printStatistics();

        // Desativa o ExitPoll para que o Pollster possa terminar
        exitPollStub.deactivate();

        // Espera que o Pollster termine
        try {
            pollsterThread.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        if (logWriter != null) {
            logWriter.close();
        }        
        log("ClientElectionDay terminou com sucesso.");
    }
}
