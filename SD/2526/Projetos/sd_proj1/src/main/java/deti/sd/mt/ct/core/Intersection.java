package deti.sd.mt.ct.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import deti.sd.mt.ct.model.Collision;
import deti.sd.mt.ct.model.Direction;
import deti.sd.mt.ct.model.MoveType;
import deti.sd.mt.ct.model.VehiclePriority;

public class Intersection {
    private static final Logger logger = LoggerFactory.getLogger(Intersection.class);
    private static final AtomicInteger ID_GENERATOR = new AtomicInteger(0);

    public final int id = ID_GENERATOR.getAndIncrement();

    /// Contains the number of vehicle inside the intersection.
    private AtomicInteger entryCount = new AtomicInteger(0);
    private final AtomicInteger waitingEmergencyCount = new AtomicInteger(0); // contador de veiculos de emergencia a espera
    private ChargingStation station = null;

    private final ReentrantLock northEntry = new ReentrantLock(true);
    private final ReentrantLock southEntry = new ReentrantLock(true);
    private final ReentrantLock eastEntry = new ReentrantLock(true);
    private final ReentrantLock westEntry = new ReentrantLock(true);

    // lock e condition para gerir o estado de ocupacao da intersecao
    // veiculos esperam na condition ate nao haver conflitos
    // fair=true para prevenir starvation sob alta contencao
    private final ReentrantLock stateLock = new ReentrantLock(true);
    private final Condition noConflict = stateLock.newCondition();

    private Direction northVehicleEntry;
    private Direction southVehicleEntry;
    private Direction easthVehicleEntry;
    private Direction westhVehicleEntry;

    private MoveType northVehicleMove;
    private MoveType southVehicleMove;
    private MoveType eastVehicleMove;
    private MoveType westVehicleMove;

    public Intersection() {
        this.northVehicleEntry = null;
        this.southVehicleEntry = null;
        this.easthVehicleEntry = null;
        this.westhVehicleEntry = null;

        this.northVehicleMove = null;
        this.southVehicleMove = null;
        this.eastVehicleMove = null;
        this.westVehicleMove = null;
    }

    /// A vehicle must call this function to enter the intersection.
    ///
    /// YOU MUST USE THIS FUNCTION
    public void enter(Direction entry, MoveType move, String vehicleId, VehiclePriority priority) {
        // DO NOT REMOVE THIS LOG
        // Must be the first instruction.
        logger.info(">| I:{} (?) V:{} E:{}, M:{}", this.id, vehicleId, entry, move);

        int n = -1;

        // lock da direcao de entrada (um veiculo por direcao de cada vez)
        getEntryLock(entry).lock();

        // Se for EMERGENCIA, sinaliza que esta a espera para que os normais cedam
        if (priority == VehiclePriority.EMERGENCY) {
            waitingEmergencyCount.incrementAndGet();
        }

        // DO NOT REMOVE THIS LOG
        // NOTE: Must be shown when waiting for the resources.
        logger.info(">< I:{} (?) V:{} E:{}, M:{}", this.id, vehicleId, entry, move);

        // esperar ate nao haver veiculos conflitantes dentro da intersecao
        stateLock.lock();
        try {
            while (true) {
                boolean hasConflict = conflictsWithCurrentOccupants(entry, move);
                
                if (priority == VehiclePriority.EMERGENCY) {
                    // veiculo de emergencia so espera se houver conflito fisico
                    if (!hasConflict) break;
                } else {
                    // veiculo normal espera por conflitos ou se houver emergencia na fila
                    if (!hasConflict && waitingEmergencyCount.get() == 0) break;
                }

                noConflict.await();
            }

            // Uma vez que tem autorizacao para entrar, se era emergencia, retira-se da contagem de espera
            if (priority == VehiclePriority.EMERGENCY) {
                waitingEmergencyCount.decrementAndGet();
            }

            setVehicleState(entry, entry, move);
        } catch (InterruptedException e) {
            if (priority == VehiclePriority.EMERGENCY) {
                waitingEmergencyCount.decrementAndGet();
            }
            Thread.currentThread().interrupt();
            getEntryLock(entry).unlock();
            return;
        } finally {
            stateLock.unlock();
        }

        // NOTE: once you acquire the entry resource, increment the number of
        // vehicles in the intersection and log the event (i.e.,`entryCount`).
        n = entryCount.incrementAndGet();

        // DO NOT REMOVE THIS LOG
        // Must be the last instruction of the enter function.
        // It signals that the vehicle has the resources to traverse the intersection.
        logger.info(">> I:{} ({}) V:{} E:{}, M:{}", this.id, n, vehicleId, entry, move);
    }

    /// A vehicle must call this function to exit the intersection.
    ///
    /// YOU MUST USE THIS FUNCTION
    public void exit(Direction entry, MoveType move, String vehicleId) {

        // limpar estado e notificar veiculos a espera
        stateLock.lock();
        try {
            setVehicleState(entry, null, null);
            noConflict.signalAll();
        } finally {
            stateLock.unlock();
        }

        getEntryLock(entry).unlock();

        // DO NOT REMOVE THIS LOG AND DECREMENT
        // Must be the last instructions of the exit function.
        int n = entryCount.decrementAndGet();
        logger.info("<< I:{} ({}) V:{} E:{}, M:{}", this.id, n, vehicleId, entry, move);
    }

    /// Sets the charging stations for this intersection.
    public void setChargingStation(ChargingStation station) {
        this.station = station;
        logger.info("Intersection {} now has charging station {}", id, station.id);
    }

    /// Returns the charging stations of this intersection.
    public ChargingStation getChargingStation() {
        return station;
    }

    /// Check if this intersection has a charging station.
    public boolean hasChargingStation() {
        return station != null;
    }

    /// number of vehicles inside this intersection.
    public int getEntryCount() {
        return entryCount.get();
    }

    private ReentrantLock getEntryLock(Direction dir) {
        return switch (dir) {
            case NORTH -> northEntry;
            case SOUTH -> southEntry;
            case EAST -> eastEntry;
            case WEST -> westEntry;
        };
    }

    // verifica se ha algum veiculo dentro da intersecao com trajeto conflitante
    private boolean conflictsWithCurrentOccupants(Direction entry, MoveType move) {
        if (Collision.collides(entry, move, northVehicleEntry, northVehicleMove))
            return true;
        if (Collision.collides(entry, move, southVehicleEntry, southVehicleMove))
            return true;
        if (Collision.collides(entry, move, easthVehicleEntry, eastVehicleMove))
            return true;
        if (Collision.collides(entry, move, westhVehicleEntry, westVehicleMove))
            return true;
        return false;
    }

    private void setVehicleState(Direction lane, Direction entryDir, MoveType moveType) {
        switch (lane) {
            case NORTH -> {
                setNorthVehicleEntry(entryDir);
                setNorthVehicleMove(moveType);
            }
            case SOUTH -> {
                setSouthVehicleEntry(entryDir);
                setSouthVehicleMove(moveType);
            }
            case EAST -> {
                setEastVehicleEntry(entryDir);
                setEastVehicleMove(moveType);
            }
            case WEST -> {
                setWestVehicleEntry(entryDir);
                setWestVehicleMove(moveType);
            }
        }
    }

    public Direction getNorthVehicleEntry() {
        return northVehicleEntry;
    }

    public void setNorthVehicleEntry(Direction northVehicleEntry) {
        this.northVehicleEntry = northVehicleEntry;
    }

    public Direction getSouthVehicleEntry() {
        return southVehicleEntry;
    }

    public void setSouthVehicleEntry(Direction southVehicleEntry) {
        this.southVehicleEntry = southVehicleEntry;
    }

    public Direction getEastVehicleEntry() {
        return easthVehicleEntry;
    }

    public void setEastVehicleEntry(Direction easthVehicleEntry) {
        this.easthVehicleEntry = easthVehicleEntry;
    }

    public Direction getWestVehicleEntry() {
        return westhVehicleEntry;
    }

    public void setWestVehicleEntry(Direction westhVehicleEntry) {
        this.westhVehicleEntry = westhVehicleEntry;
    }

    public MoveType getNorthVehicleMove() {
        return northVehicleMove;
    }

    public void setNorthVehicleMove(MoveType northVehicleMove) {
        this.northVehicleMove = northVehicleMove;
    }

    public MoveType getSouthVehicleMove() {
        return southVehicleMove;
    }

    public void setSouthVehicleMove(MoveType southVehicleMove) {
        this.southVehicleMove = southVehicleMove;
    }

    public MoveType getEastVehicleMove() {
        return eastVehicleMove;
    }

    public void setEastVehicleMove(MoveType eastVehicleMove) {
        this.eastVehicleMove = eastVehicleMove;
    }

    public MoveType getWestVehicleMove() {
        return westVehicleMove;
    }

    public void setWestVehicleMove(MoveType westVehicleMove) {
        this.westVehicleMove = westVehicleMove;
    }
}
