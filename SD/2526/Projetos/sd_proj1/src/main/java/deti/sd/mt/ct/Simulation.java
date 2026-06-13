package deti.sd.mt.ct;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import picocli.CommandLine;
import picocli.CommandLine.Option;
import picocli.CommandLine.Command;

import deti.sd.mt.ct.core.CityMap;
import deti.sd.mt.ct.core.Intersection;
import deti.sd.mt.ct.core.Vehicle;
import deti.sd.mt.ct.model.VehiclePriority;
import deti.sd.mt.ct.ui.SimulationGUI;

@Command(name = "mt-ct", mixinStandardHelpOptions = true, version = "25'26")
public class Simulation implements Runnable {

    @Option(names = { "-t", "--time" }, description = "Time unit (ms)")
    private int timeUnit = 1;

    @Option(names = { "-r", "--rows" }, description = "City Map rows")
    private int rows = 5;

    @Option(names = { "-c", "--cols" }, description = "City Map columns")
    private int cols = 5;

    @Option(names = { "-s", "--steps" }, description = "Number of simulation steps")
    private int steps = 10;

    @Option(names = { "-v", "--vehicle" }, description = "Number of vehicles")
    private int nvehicles = 5;

    @Option(names = { "-g", "--gui" }, description = "Run with a GUI")
    private boolean showGui;

    private void validateArguments() {
        CommandLine cmd = new CommandLine(this);

        if (timeUnit < 1) {
            throw new CommandLine.ParameterException(cmd, "--time must be >= 1");
        }
        if (rows < 1) {
            throw new CommandLine.ParameterException(cmd, "--rows must be >= 1");
        }
        if (cols < 1) {
            throw new CommandLine.ParameterException(cmd, "--cols must be >= 1");
        }
        if (steps < 1) {
            throw new CommandLine.ParameterException(cmd, "--steps must be >= 1");
        }
        if (nvehicles < 1) {
            throw new CommandLine.ParameterException(cmd, "--vehicle must be >= 1");
        }
    }

    @Override
    public void run() {
        validateArguments();

        // 1. Initialize the Grid Map
        CityMap map = new CityMap(rows, cols);

        // 2. Spwan the Charging Stations
        map.spawnChargingStations(nvehicles, timeUnit);

        // 3. Launch the vehicles
        Random random = new Random();
        List<Thread> vehicleThreads = new ArrayList<>();
        List<Vehicle> vehicles = new ArrayList<>();

        for (int i = 0; i < nvehicles; i++) {
            int r = random.nextInt(rows);
            int c = random.nextInt(cols);
            Intersection start = map.getIntersection(r, c);
            // define a prioridade (10% de chance de ser emergencia)
            VehiclePriority priority;
            if (random.nextDouble() < 0.1) {
                priority = VehiclePriority.EMERGENCY;
            } else {
                priority = VehiclePriority.NORMAL;
            }
            Vehicle vehicle = new Vehicle(steps, start, map, timeUnit, priority);
            vehicles.add(vehicle);
            Thread t = new Thread(vehicle, vehicle.id);
            vehicleThreads.add(t);
        }

        // Start all vehicle threads
        for (Thread t : vehicleThreads) {
            t.start();
        }

        // 4. Show GUI, if requested
        if (showGui) {
            SimulationGUI gui = new SimulationGUI(map, vehicles);
            gui.show();
        }

        // 5. Wait for the vehicles threads to terminate
        for (Thread t : vehicleThreads) {
            try {
                t.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    public static void main(String[] args) {
        int exitCode = new CommandLine(new Simulation()).execute(args);
        System.exit(exitCode);
    }
}
