package deti.sd.mt.ct.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Queue;

import deti.sd.mt.ct.model.Coordinate;

public class CityMap {
    private static final Logger logger = LoggerFactory.getLogger(CityMap.class);

    private final int rows;
    private final int cols;

    private final List<Intersection> grid = new ArrayList<>();

    public CityMap(int rows, int cols) {
        this.rows = rows;
        this.cols = cols;

        for (int i = 0; i < rows * cols; i++) {
            grid.add(new Intersection());
        } // end for

        // DO NOT REMOVE THIS LOG
        // Must be the last instruction.
        logger.info("New CityMap with {} rows and {} columns", rows, cols);
    }

    /// Add the `station` to the intersection `isec`
    public void addChargingStation(Intersection isec, ChargingStation station) {
        isec.setChargingStation(station);
    }

    /// Spawnar charging stations in the city map.
    public void spawnChargingStations(int numVehicles, int timeUnit) {
        int totalIntersections = rows * cols;
        // int maxStations = totalIntersections / 4;
        // int numStations = Math.min(maxStations, totalIntersections / 5);
        int numStations = totalIntersections / 4;
        int numPlugs = Math.max(1, numVehicles / 4);

        List<Integer> indices = new ArrayList<>();
        for (int i = 0; i < totalIntersections; i++) {
            indices.add(i);
        }
        Collections.shuffle(indices);

        int stationGridRows = Math.max(1, (int) Math.round(Math.sqrt(numStations * (rows / (double) cols))));
        int stationGridCols = Math.max(1, (int) Math.ceil(numStations / (double) stationGridRows));

        int spawnedStations = 0;
        for (int gr = 0; gr < stationGridRows && spawnedStations < numStations; gr++) {
            int r = Math.min(rows - 1, (int) (((gr + 0.5) * rows) / stationGridRows));
            for (int gc = 0; gc < stationGridCols && spawnedStations < numStations; gc++) {
                int c = Math.min(cols - 1, (int) (((gc + 0.5) * cols) / stationGridCols));
                Intersection isec = getIntersection(r, c);
                if (isec == null || isec.getChargingStation() != null) {
                    continue;
                }

                ChargingStation station = new ChargingStation(numPlugs, timeUnit);
                addChargingStation(isec, station);
                spawnedStations++;
            }
        }

        if (spawnedStations < numStations) {
            for (int i = 0; i < totalIntersections && spawnedStations < numStations; i++) {
                Intersection isec = grid.get(i);
                if (isec.getChargingStation() != null) {
                    continue;
                }
                ChargingStation station = new ChargingStation(numPlugs, timeUnit);
                addChargingStation(isec, station);
                spawnedStations++;
            }
        }

        logger.info("Spawned {} charging stations ({} plugs each) on a {}x{} grid",
                numStations, numPlugs, rows, cols);
    }

    /// Returns the total number of rows in the city grid.
    public int getRows() {
        return rows;
    }

    /// Returns the total number of cols in the city grid.
    public int getCols() {
        return cols;
    }

    /// Returns the intersection located at row `r` and column `c`.
    public Intersection getIntersection(int r, int c) {
        if (r >= 0 && c >= 0 && r < rows && c < cols)
            return grid.get(c + cols * r);
        return null;
    }

    /// Returns the intersection object by `id`.
    public Intersection getIntersection(int id) {
        if (id >= 0 && id < rows * cols)
            return grid.get(id);
        return null;
    }

    /// Returns the intersection object located at coordinate `coord`.
    public Intersection getIntersection(Coordinate coord) {
        int idx = coord.x() + coord.y() * cols;
        if (idx >= 0 && idx < rows * cols)
            return grid.get(idx);
        return null;
    }

    /// Returns the coordinates of an intersection.
    public Coordinate getCoordinate(Intersection isec) {
        return new Coordinate(isec.id % cols, isec.id / cols);
    }

    /// Returns the nearest intersection to the `current` intersection.
    public Intersection findNearestChargerIntersection(Intersection current) {

        Coordinate begin = getCoordinate(current);
        int totalRows = getRows();
        int totalCols = getCols();

        boolean[][] visited = new boolean[totalRows][totalCols];
        Queue<int[]> queue = new ArrayDeque<>();

        int beginRow = begin.y();
        int beginCol = begin.x();
        visited[beginRow][beginCol] = true;
        queue.offer(new int[] { beginRow, beginCol });

        int[][] possibleDirections = { { 0, 1 }, { 0, -1 }, { 1, 0 }, { -1, 0 } };

        while (!queue.isEmpty()) {
            int[] pos = queue.poll();
            int r = pos[0], c = pos[1];
            Intersection isec = getIntersection(r, c);
            if (isec != null && isec.getChargingStation() != null) {
                return isec;
            }
            for (int[] d : possibleDirections) {
                int nr = r + d[0], nc = c + d[1];
                if (nr >= 0 && nr < totalRows && nc >= 0 && nc < totalCols && !visited[nr][nc]) {
                    visited[nr][nc] = true;
                    queue.offer(new int[] { nr, nc });
                }
            }
        }

        return null;
    }
}
