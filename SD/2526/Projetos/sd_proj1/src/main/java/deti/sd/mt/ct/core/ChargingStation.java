package deti.sd.mt.ct.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import deti.sd.mt.ct.model.VehiclePriority;

public class ChargingStation {
	private static final Logger logger = LoggerFactory.getLogger(ChargingStation.class);
	private static final AtomicInteger ID_GENERATOR = new AtomicInteger(0);

	public final int id = ID_GENERATOR.getAndIncrement();
	public static final int CHARGING_SIMULATION_TIME_MIN = 10;
	public static final int CHARGING_SIMULATION_TIME_MAX = 50;
	public static final int POWER_AVAILABLE_UNITS_MIN = 50;
	public static final int POWER_AVAILABLE_UNITS_MAX = 80;

	private final int timeUnit;
	private final int numPlugs;
	private int availablePlugs;
	private int availablePower;

	private final AtomicInteger waitingEmergencyCount = new AtomicInteger(0); // contador de veiculos de emergencia a espera

	// fair=true para prevenir starvation no acesso aos chargers
	private final ReentrantLock chargingLock = new ReentrantLock(true);
	private final Condition resourcesAvailable = chargingLock.newCondition();

	public ChargingStation(int numPlugs, int timeUnit) {

		this.numPlugs = numPlugs;
		this.timeUnit = timeUnit;
		this.availablePlugs = numPlugs;
		this.availablePower = 100 + (numPlugs - 1)
				* ThreadLocalRandom.current().nextInt(POWER_AVAILABLE_UNITS_MIN, POWER_AVAILABLE_UNITS_MAX + 1);

		// DO NOT REMOVE THIS LOG
		// Must be the last instruction
		logger.info("New charging station with id:{} plugs:{} power:{}",
				this.id, numPlugs, availablePower);
	}

	public void useCharger(String vehicleId, int amountNeeded, VehiclePriority priority) {

		if (priority == VehiclePriority.EMERGENCY) {
			waitingEmergencyCount.incrementAndGet();
		}

		// 1. esperar por recursos e adquiri-los atomicamente (mesmo bloco de lock)
		chargingLock.lock();
		try {
			// DO NOT REMOVE THIS LOG
			// Must be the first instruction after enter the critical region
			logger.info(">| C:{} V:{} S:{} P:{} A:{}", this.id, vehicleId, availablePlugs, amountNeeded,
					this.availablePower);

			// DO NOT REMOVE THIS LOG
			// NOTE: Must be shown when waiting for the resources.
			logger.info(">< C:{} V:{} S:{} P:{} A:{}", this.id, vehicleId, availablePlugs, amountNeeded,
					this.availablePower);

			// esperar ate haver plug E potencia suficiente
			while (true) {
				boolean resourcesAvailableLocal = availablePlugs >= 1 && availablePower >= amountNeeded;

				if (priority == VehiclePriority.EMERGENCY) {
					// emergencia so espera por recursos disponiveis
					if (resourcesAvailableLocal)
						break;
				} else {
					// normal espera por recursos E por nao haver emergencias na fila
					if (resourcesAvailableLocal && waitingEmergencyCount.get() == 0)
						break;
				}
				resourcesAvailable.await();
			}

			if (priority == VehiclePriority.EMERGENCY) {
				waitingEmergencyCount.decrementAndGet();
			}

			// 2. adquirir recursos (ainda dentro do mesmo lock, sem race condition)
			availablePlugs -= 1;
			availablePower -= amountNeeded;

			// DO NOT REMOVE THIS LOG
			// NOTE: Must be shown after acquiring the resources
			logger.info(">> C:{} V:{} S:{} P:{} A:{}", this.id, vehicleId, availablePlugs, amountNeeded,
					this.availablePower);
		} catch (InterruptedException e) {
			if (priority == VehiclePriority.EMERGENCY) {
				waitingEmergencyCount.decrementAndGet();
			}
			Thread.currentThread().interrupt();
			return;
		} finally {
			chargingLock.unlock();
		}

		// 3. Simulate charging (outside the lock so other vehicles can acquire plugs)
		chargingSimulation();

		// 4. Release the resources
		chargingLock.lock();
		try {
			availablePlugs += 1;
			availablePower += amountNeeded;
			resourcesAvailable.signalAll();

			// DO NOT REMOVE THIS LOG
			// NOTE: Must be shown after releasing the resources
			logger.info("<< C:{} V:{} S:{} P:{} A:{}", this.id, vehicleId, availablePlugs, amountNeeded,
					this.availablePower);
		} finally {
			chargingLock.unlock();
		}
	}

	public void chargingSimulation() {

		int chargingTime = ThreadLocalRandom.current().nextInt(CHARGING_SIMULATION_TIME_MIN,
				CHARGING_SIMULATION_TIME_MAX + 1);

		try {
			Thread.sleep((long) chargingTime * timeUnit);
		} catch (InterruptedException e) {
			Thread.currentThread().interrupt();
			return;
		}
	}

	/// Returns the current available power
	public int getPowerLevel() {
		return availablePower;

	}

	public int getAvailablePlugs() {
		return availablePlugs;
	}

	public void setPowerLevel(int availablePower) {
		this.availablePower = availablePower;
	}

	public void setAvailablePlugs(int availablePlugs) {
		this.availablePlugs = availablePlugs;
	}

}
