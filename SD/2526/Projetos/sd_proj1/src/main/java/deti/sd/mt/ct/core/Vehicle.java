package deti.sd.mt.ct.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.atomic.AtomicInteger;

import deti.sd.mt.ct.model.Direction;
import deti.sd.mt.ct.model.MoveType;
import deti.sd.mt.ct.model.Coordinate;
import deti.sd.mt.ct.model.VehiclePriority;

// notas do guiao sobre o vehicle:
// cada vehicle e uma thread independente (implements runnable)
// inicializado com intersecao e direcao aleatorias, e numero maximo de steps
// loop de execucao continua ate acabar os steps ou bateria chegar a 0%
// bateria comeca a 100%, power requirement entre 50 e 100 (uniforme)
// consumo de viagem entre intersecoes: 2% a 5%, consumo ao atravessar intersecao: 1% a 2%
// tempo de viagem entre intersecoes: 10 a 50 unidades de tempo (sleep)
// tempo de travessia de intersecao: 5 a 25 unidades de tempo (sleep)
// protocolo enter-traverse-exit ao interagir com intersecoes
// se bateria cai abaixo do threshold (20%), interromper destino atual e ir ao charger mais proximo
// apos carregar (bateria volta a 100%), retomar o destino original
// ao chegar ao destino, comecar novo ciclo (escolher novo destino aleatorio)
// nao usar synchronized, apenas java.util.concurrent

// extra 1
// priorities normal & emergy
// há o enum vehiclepriority (podemos usar um bool mas vou implementar com isto que distingue NORMAL & EMERGENCY)
// adicionar a prioridade em vehicle (att e get) & update Simulation para dar spawn the diferentes
// adicionar priority ao metodo enter 

public class Vehicle implements Runnable {
    private static final Logger logger = LoggerFactory.getLogger(Vehicle.class);
    private static final AtomicInteger ID_GENERATOR = new AtomicInteger(0);

    public final String id;

    private final int numSteps;
    private final CityMap map;
    private final int timeUnit;
    private int step;
    private int battery;

    private VehiclePriority priority;

    private Coordinate currentPosition;
    private Direction currentDirection;
    private Coordinate destination;
    private Coordinate originalDestination;
    private List<Coordinate> path;
    private int pathIndex;
    private boolean chargingDiversion;

    private final int powerRequirement;

    private static final int VEHICLE_BATTERY_THRESHOLD_PERCENTAGE = 40;

    private static final int VEHICLE_POWER_REQUIREMENT_UNITS_MIN = 50;
    private static final int VEHICLE_POWER_REQUIREMENT_UNITS_MAX = 100;

    private static final int VEHICLE_INTERSECTION_TRAVERSE_COST_PERCENTAGE_MIN = 1;
    private static final int VEHICLE_INTERSECTION_TRAVERSE_COST_PERCENTAGE_MAX = 2;
    private static final int VEHICLE_INTERSECTION_TRAVERSE_TIME_MIN = 5;
    private static final int VEHICLE_INTERSECTION_TRAVERSE_TIME_MAX = 25;

    private static final int VEHICLE_INTER_INTERSECTION_TRAVERSE_COST_PERCENTAGE_MIN = 2;
    private static final int VEHICLE_INTER_INTERSECTION_TRAVERSE_COST_PERCENTAGE_MAX = 5;
    private static final int VEHICLE_INTER_INTERSECTION_TRAVERSE_TIME_MIN = 10;
    private static final int VEHICLE_INTER_INTERSECTION_TRAVERSE_TIME_MAX = 50;

    private volatile boolean finished = false;
    private volatile boolean charging = false;

    public Vehicle(int numSteps, Intersection start, CityMap map, int timeUnit, VehiclePriority priority) {
        this.numSteps = numSteps;
        this.map = map;
        this.timeUnit = timeUnit;
        this.priority = priority;
        // prefixo do ID depende da prioridade
        int numeroId = ID_GENERATOR.getAndIncrement();
        if (priority == VehiclePriority.EMERGENCY) {
            this.id = "Emergency-" + numeroId;
        } else {
            this.id = "Vehicle-" + numeroId;
        }
        this.step = 0;
        this.battery = 100;

        this.currentPosition = map.getCoordinate(start);
        this.currentDirection = Direction.values()[ThreadLocalRandom.current().nextInt(4)]; // direcao inicial aleatoria

        this.powerRequirement = ThreadLocalRandom.current().nextInt(VEHICLE_POWER_REQUIREMENT_UNITS_MIN,
                VEHICLE_POWER_REQUIREMENT_UNITS_MAX + 1); // entre 50 e 100 unidades de potencia

        this.destination = null;
        this.originalDestination = null;
        this.path = null;
        this.pathIndex = 0;
        this.chargingDiversion = false;

        // DO NOT REMOVE THIS LOG
        // Must be the last instruction.
        logger.info("New vehicle with id '{}' at intersection {}", id, start.id);
    }

    @Override
    public void run() {
        // DO NOT REMOVE THIS LOG
        // Must be the first instruction.
        logger.info("Vehicle with id '{}' has started", id);

        // continua enquanto houver steps e bateria > 0
        while ((step < numSteps) && (battery > 0)) {
            // DO NOT REMOVE THIS LOG
            // Must be the first instruction of the while loop.
            logger.info("SoS ({}) P:{}", step + 1, battery);

            // se o caminho acabou ou nao existe, decidir proximo passo
            if (path == null || pathIndex >= path.size()) {
                // se estava a desviar para carregar, usar o charger e voltar ao destino oiginal
                if (chargingDiversion) {
                    Intersection chargerIsec = map.getIntersection(currentPosition);
                    if (chargerIsec != null && chargerIsec.hasChargingStation()) {
                        charging = true;
                        chargerIsec.getChargingStation().useCharger(id, powerRequirement, priority); // bloqueia ate ter plug e
                                                                                           // potencia
                        charging = false;
                        battery = 100; // apos carregar, bateria volta a 100
                    }
                    chargingDiversion = false;

                    // retomar destino original apos carregar
                    if (originalDestination != null) {
                        destination = originalDestination;
                        originalDestination = null;
                        path = computePath(currentPosition, destination);
                        pathIndex = 0;
                    }
                }

                // se ainda nao tem caminho, escolher novo destino aleatorio
                if (path == null || pathIndex >= path.size()) {
                    selectNewDestination();
                }
            }

            // verificar se bateria esta abaixo do threshold e precisa de desviar para
            // charger
            if (!chargingDiversion && battery < VEHICLE_BATTERY_THRESHOLD_PERCENTAGE) {
                Intersection currentIsec = map.getIntersection(currentPosition);
                Intersection nearest = map.findNearestChargerIntersection(currentIsec); // charger mais proximo
                if (nearest != null) {
                    Coordinate chargerCoord = map.getCoordinate(nearest);
                    if (!chargerCoord.equals(currentPosition)) {
                        // guardar destino original e desviar para o charger
                        originalDestination = destination;
                        destination = chargerCoord;
                        path = computePath(currentPosition, destination);
                        pathIndex = 0;
                        chargingDiversion = true;
                    } else {
                        // ja ta no charger, carregar imediatamente
                        charging = true;
                        nearest.getChargingStation().useCharger(id, powerRequirement, priority);
                        charging = false;
                        battery = 100;
                    }
                }
            }

            // avancar uma intersecao no caminho
            if (path != null && pathIndex < path.size()) {
                Coordinate nextPos = path.get(pathIndex);

                // calcular direcao de viagem e direcao de entrada na intersecao (oposta)
                Direction travelDir = travelDirection(currentPosition, nextPos);
                Direction entryDir = travelDir.opposite();

                // calcular direcao de saida para determinar o tipo de movimento (straight,
                // left, right, u-turn)
                Direction exitDir;
                if (pathIndex + 1 < path.size()) {
                    exitDir = travelDirection(nextPos, path.get(pathIndex + 1));
                } else {
                    exitDir = travelDir;
                }
                MoveType moveType = computeMoveType(entryDir, exitDir);

                Intersection nextIsec = map.getIntersection(nextPos);

                // viagem entre intersecoes - 10 a 50 unidades de tempo, custo de 2 a 5 de
                // bateria
                int travelTime = ThreadLocalRandom.current().nextInt(VEHICLE_INTER_INTERSECTION_TRAVERSE_TIME_MIN,
                        VEHICLE_INTER_INTERSECTION_TRAVERSE_TIME_MAX + 1);
                int travelCost = ThreadLocalRandom.current().nextInt(
                        VEHICLE_INTER_INTERSECTION_TRAVERSE_COST_PERCENTAGE_MIN,
                        VEHICLE_INTER_INTERSECTION_TRAVERSE_COST_PERCENTAGE_MAX + 1);
                try {
                    Thread.sleep((long) travelTime * timeUnit);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    return;
                }
                battery = Math.max(0, battery - travelCost);

                // se bateria chegou a 0, terminar execucao no fim do step
                if (battery <= 0) {
                    logger.info("EoS ({}) P:{}", step + 1, battery);
                    step = step + 1;
                    break;
                }

                // protocolo enter-traverse-exit
                nextIsec.enter(entryDir, moveType, id, priority);

                // travessia da intersecao - 5 a 25 unidades de tempo, custo de 1 a 2 de bateria
                int traversalTime = ThreadLocalRandom.current().nextInt(VEHICLE_INTERSECTION_TRAVERSE_TIME_MIN,
                        VEHICLE_INTERSECTION_TRAVERSE_TIME_MAX + 1);
                int traversalCost = ThreadLocalRandom.current().nextInt(
                        VEHICLE_INTERSECTION_TRAVERSE_COST_PERCENTAGE_MIN,
                        VEHICLE_INTERSECTION_TRAVERSE_COST_PERCENTAGE_MAX + 1);
                try {
                    Thread.sleep((long) traversalTime * timeUnit);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    nextIsec.exit(entryDir, moveType, id);
                    return;
                }
                battery = Math.max(0, battery - traversalCost);

                // exit - liberta a intersecao e notifica threads a espera
                nextIsec.exit(entryDir, moveType, id);

                // atualizar posicao e direcao apos movimento
                currentPosition = nextPos;
                currentDirection = travelDir;
                pathIndex++;
            }

            // DO NOT REMOVE THIS LOG AND INCREMENT
            // Must be the last instructions of the while loop.
            logger.info("EoS ({}) P:{}", step + 1, battery);
            step = step + 1;
        } // end while (steps and battery)
        finished = true;
    }

    // escolher destino aleatorio diferente da posicao atual e calcular caminho
    private void selectNewDestination() {
        int rows = map.getRows();
        int cols = map.getCols();

        if (rows == 1 && cols == 1) {
            destination = currentPosition;
            path = new ArrayList<>();
            pathIndex = 0;
            return;
        }

        Coordinate newDest;
        do {
            int x = ThreadLocalRandom.current().nextInt(cols);
            int y = ThreadLocalRandom.current().nextInt(rows);
            newDest = new Coordinate(x, y);
        } while (newDest.equals(currentPosition));

        destination = newDest;
        path = computePath(currentPosition, destination);
        pathIndex = 0;
    }

    // pathfinding simples - primeiro move em x, depois em y (manhattan)
    private List<Coordinate> computePath(Coordinate from, Coordinate to) {
        List<Coordinate> result = new ArrayList<>();
        int x = from.x(), y = from.y();
        int tx = to.x(), ty = to.y();

        while (x != tx) {
            x += (tx > x) ? 1 : -1;
            result.add(new Coordinate(x, y));
        }
        while (y != ty) {
            y += (ty > y) ? 1 : -1;
            result.add(new Coordinate(x, y));
        }
        return result;
    }

    // calcula a direcao cardinal
    private static Direction travelDirection(Coordinate from, Coordinate to) {
        int dx = to.x() - from.x();
        int dy = to.y() - from.y();
        if (dx > 0)
            return Direction.EAST;
        if (dx < 0)
            return Direction.WEST;
        if (dy > 0)
            return Direction.SOUTH;
        if (dy < 0)
            return Direction.NORTH;
        return Direction.NORTH;
    }

    // usado para verificar colisoes na intersecao
    private static MoveType computeMoveType(Direction entry, Direction exitDir) {
        if (entry == exitDir)
            return MoveType.U_TURN;
        if (entry.opposite() == exitDir)
            return MoveType.STRAIGHT;
        return switch (entry) {
            case NORTH -> exitDir == Direction.EAST ? MoveType.LEFT_TURN : MoveType.RIGHT_TURN;
            case SOUTH -> exitDir == Direction.WEST ? MoveType.LEFT_TURN : MoveType.RIGHT_TURN;
            case EAST -> exitDir == Direction.SOUTH ? MoveType.LEFT_TURN : MoveType.RIGHT_TURN;
            case WEST -> exitDir == Direction.NORTH ? MoveType.LEFT_TURN : MoveType.RIGHT_TURN;
        };
    }

    public boolean isFinished() {
        return finished;
    }

    public boolean isCharging() {
        return charging;
    }

    public boolean isChargingDiversion() {
        return chargingDiversion;
    }

    // visual position: column (x) and row (y) for the gui simulation
    public double getVisualX() {
        return currentPosition.x();
    }

    public double getVisualY() {
        return currentPosition.y();
    }

    public Coordinate getCurrentPosition() {
        return currentPosition;
    }

    public Direction getCurrentDirection() {
        return currentDirection;
    }

    public int getBattery() {
        return battery;
    }

    public int getPowerRequirement() {
        return powerRequirement;
    }

    public int getStep() {
        return step;
    }

    public VehiclePriority getPriority() {
        return priority;
    }
}
