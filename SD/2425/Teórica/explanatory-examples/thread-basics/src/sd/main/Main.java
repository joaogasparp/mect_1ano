package sd.main;

import genclass.GenericIO;
import sd.env.CollectEnvironmentData;
import sd.hierarchy.ThreadHierarchy1;
import sd.info.CollectThreadData;
import sd.preempt.TestPreemptiveness;
import sd.run.ListWorkDir;

/**
 * Frontend launcher for all SD package main classes.
 */
public class Main {
    /**
     * Main program that allows launching other main classes.
     *
     * @param args runtime parameter list
     */
    public static void main(String[] args) {
        // while (true) {
            showMenu();
            int choice = GenericIO.readlnInt();

            switch (choice) {
                case 0:
                    GenericIO.writelnString("Exiting program...");
                    System.exit(0);
                    break;
                case 1:
                    GenericIO.writelnString("\nRunning CollectEnvironmentData...\n");
                    CollectEnvironmentData.main(args);
                    break;
                case 2:
                    GenericIO.writelnString("\nRunning ThreadHierarchy1...\n");
                    ThreadHierarchy1.main(args);
                    break;
                case 3:
                    GenericIO.writelnString("\nRunning CollectThreadData...\n");
                    CollectThreadData.main(args);
                    break;
                case 4:
                    GenericIO.writelnString("\nRunning TestPreemptiveness...\n");
                    TestPreemptiveness.main(args);
                    break;
                case 5:
                    GenericIO.writelnString("\nRunning ListWorkDir...\n");
                    ListWorkDir.main(args);
                    break;
                default:
                    GenericIO.writelnString("Invalid option! Please try again.");
            }

            // GenericIO.writelnString("\nPress ENTER to continue...");
            // GenericIO.readlnString();
        // }
    }

    /**
     * Display the menu of available programs to run.
     */
    private static void showMenu() {
        GenericIO.writelnString("\n=== SD Package Main Programs ===");
        GenericIO.writelnString("1. CollectEnvironmentData - Show JVM and environment info");
        GenericIO.writelnString("2. ThreadHierarchy1 - Demonstrate thread hierarchy");
        GenericIO.writelnString("3. CollectThreadData - Show thread characteristics");
        GenericIO.writelnString("4. TestPreemptiveness - Test thread preemptiveness");
        GenericIO.writelnString("5. ListWorkDir - List working directory contents");
        GenericIO.writelnString("0. Exit");
        GenericIO.writeString("\nChoose an option: ");
    }
}
