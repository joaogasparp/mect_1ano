# MOSS - Microservice-oriented Object Storage Service

MOSS is a distributed object storage system designed with a microservices architecture.
It provides a scalable solution for storing and retrieving binary objects over a network using gRPC.

## Architecture

The system consists of several specialized node types:

*   **Manager Node:** The central coordinator. It maintains a registry of available Volume Nodes and manages file placement.
*   **Volume Node:** The storage layer. These nodes store raw data on the local file system and provide read/write access. They send periodic heartbeats to the Manager.
*   **Object Node:** The abstraction layer. It provides an object-level API (Put/Get) to clients, coordinating with the Manager to find volumes and with Volumes to transfer data.
*   **Client CLI:** A command-line tool for users to interact with MOSS.

## Features

*   **Distributed Storage:** Data is spread across multiple volume nodes.
*   **Heartbeat Mechanism:** Manager tracks node health via periodic heartbeats.
*   **gRPC Communication:** High-performance, language-agnostic communication between nodes.
*   **Docker Support:** Easy deployment using Docker Compose.

---

## Getting Started

### Prerequisites

*   **Java 21** or higher.
*   **Maven** (or use the included `./mvnw`).
*   **Docker & Docker Compose** (optional, for containerized deployment).

### Building

To compile the project and generate gRPC stubs:

```bash
./mvnw compile
```

Alternatively, use the provided `makefile`:

```bash
make compile
```

### Running with Docker Compose (Recommended)

The easiest way to start a full cluster (1 Manager, 1 Object Node, 2 Volume Nodes) is using Docker Compose:

```bash
docker-compose up
```

The Object Node will be accessible on port `4281`.

### Running Manually

You can start individual nodes using the `./run` script:

1.  **Start the Manager:**
    ```bash
    ./run manager
    ```
2.  **Start one or more Volume Nodes:**
    ```bash
    ./run volume -d data/v1 -m localhost:4081
    ```
3.  **Start the Object Node:**
    ```bash
    ./run object -m localhost:4081
    ```

---

## Client Usage

The MOSS client provides commands to store and retrieve objects.

### Put an Object

Upload a local file to MOSS:

```bash
./run moss put <local-file> <path>
```

**Options:**
*   `-b, --bucket`: Specify a bucket name (default: `sd`).
*   `-r, --remote`: Object server address (default: `localhost:4281`).

### Get an Object

Download an object from MOSS to a local file:

```bash
./run moss get <path> <local-destination>
```

**Options:**
*   `-b, --bucket`: Specify the bucket name.
*   `-r, --remote`: Object server address (default: `localhost:4281`).

---

## Configuration

Nodes accept several command-line arguments for configuration:

| Node | Option | Description | Default |
| :--- | :--- | :--- | :--- |
| **Manager** | `-p, --port` | gRPC listening port | `4081` |
| | `-m, --mdir` | Metadata directory | `data` |
| **Volume** | `-p, --port` | gRPC listening port | `4181` |
| | `-d, --dir` | **(Required)** Storage directory | |
| | `-m, --manager` | Manager URL | `localhost:4081` |
| **Object** | `-p, --port` | gRPC listening port | `4281` |
| | `-d` | Database path | `data/obj.db` |
| | `-m, --manager` | Manager URL | `localhost:4081` |

---

## Development

### Project Structure

*   `src/main/proto/`: gRPC service definitions (`.proto`).
*   `src/main/java/deti/sd/moss/app/`: Entry points for each node type.
*   `src/main/java/deti/sd/moss/core/`: Domain logic and port definitions.
*   `src/main/java/deti/sd/moss/infra/`: Adapters for gRPC and CLI.

---

## Implementation Summary

This repository implements the MOSS assignment reference architecture with working Manager, Volume and Object nodes, a CLI client and a JavaFX monitoring dashboard.

- **Manager:** [src/main/java/deti/sd/moss/core/manager/ManagerServiceProviderImpl.java](src/main/java/deti/sd/moss/core/manager/ManagerServiceProviderImpl.java) - handles Assign, Lookup and Heartbeat; persistent ledger with H2 MVStore; ID generation and stale-node detection.
- **Volume:** [src/main/java/deti/sd/moss/core/volume/VolumeServiceProviderImpl.java](src/main/java/deti/sd/moss/core/volume/VolumeServiceProviderImpl.java) - manages local volumes, append-only data files (`<vid>.data`) and index files (`<vid>.idx`); implements heartbeats to Manager.
- **Object:** [src/main/java/deti/sd/moss/core/object/ObjectServiceProviderImpl.java](src/main/java/deti/sd/moss/core/object/ObjectServiceProviderImpl.java) - object-level API (Put/Get/List), local metadata store (MVStore) and orchestration with Manager/Volume.
- **CLI Client:** [src/main/java/deti/sd/moss/app/ClientNode.java](src/main/java/deti/sd/moss/app/ClientNode.java) and the `./run moss` helper for put/get/list commands.
- **GUI Dashboard:** [src/main/java/deti/sd/moss/infra/ui/gui/MossDashboardApp.java](src/main/java/deti/sd/moss/infra/ui/gui/MossDashboardApp.java) and supporting view/backend classes that poll the Manager state endpoint and display cluster topology and storage metrics.

## What Was Implemented

- Full gRPC-based communication between components using the supplied `.proto` contracts.
- Persistent metadata for Manager and Object nodes using H2 MVStore (`manager.db`, `obj.db`).
- Heartbeat protocol: Volumes report status every 5s; Manager treats nodes as stale after 20s without heartbeat.
- Assign/Lookup workflow: Object asks Manager for an assignment (ticket), then writes data to the chosen Volume and records metadata locally.
- Append-only volume storage: volumes store raw bytes in `<vid>.data` and an index in `<vid>.idx`.

## Synchronization Strategy

The implementation follows the assignment constraint of using `java.util.concurrent` primitives (no `synchronized`). Key choices and rationale:

- **Manager (coordinator):**
    - Uses `ConcurrentHashMap` for `localRegistry` and `nodeRegistry` to allow concurrent heartbeats and lookups without global locking. See [ManagerServiceProviderImpl.java](src/main/java/deti/sd/moss/core/manager/ManagerServiceProviderImpl.java).
    - `AtomicInteger` generators (`FILE_ID_GENERATOR`, `VOLUME_ID_GENERATOR`) provide thread-safe, monotonically increasing identifiers to avoid collisions under concurrent assigns.
    - A single-thread `ScheduledExecutorService` performs periodic stale-node checks; active operations filter by timestamp rather than deleting history, which avoids concurrent-modification races.
    - Persistence (MVStore ledger) is updated and explicitly committed immediately after ID generation to prevent reuse after restart.

- **Volume (storage node):**
    - Uses a `ReentrantLock` (`lock`) to serialize critical sections that modify the `volumes` map, append to on-disk files and update in-memory indexes. This guarantees that `onAssignVolume`, `onWrite` and `onRead` are mutually consistent and avoids interleaved file appends.
    - Stores per-volume file metadata in `ConcurrentHashMap` so concurrent reads can happen safely while writes are guarded by the `ReentrantLock`.
    - A `ScheduledExecutorService` sends heartbeats on a background thread; the heartbeat snapshot is created inside the `lock` to ensure reported metrics are consistent with ongoing writes.

- **Object (interface node):**
    - Uses `MVStore` + `MVMap` to persist object metadata and a `ReentrantReadWriteLock` to coordinate access:
        - `readLock` for `onGet` and `onList` - allows concurrent readers.
        - `writeLock` for `onPut` - ensures metadata updates (put + commit) are atomic and visible to subsequent readers.
    - This read/write separation improves throughput (multiple concurrent GETs) while preserving correctness for updates.

- **Service discovery & caches:** `GrpcServiceDiscovery` uses `ConcurrentHashMap` for channel caching to avoid races when multiple threads request the same remote stub.

Why these choices?
- `ConcurrentHashMap` and `AtomicInteger` allow lock-free common-case operations (lookups, id generation) and scale well.
- `ReentrantLock` is used where complex multi-step file-system operations must be atomic (create files, append data, update index) - a coarse-grained lock simplifies correctness and avoids subtle interleavings when mutating on-disk state.
- `ReadWriteLock` in the Object node allows many concurrent reads while still serializing writes that must persist to disk.

## Correctness Notes & Edge Cases

- ID safety: `AtomicInteger` + immediate `store.commit()` in Manager prevents fid/vid reuse across restarts.
- Stale detection: Manager considers nodes stale after 20s without heartbeat; stale nodes are ignored for Assign/Lookup.
- Conservative capacity: Manager enforces a conservative capacity rule - it only places data onto volumes with more than 10% free space (equivalent to assuming 90% usable capacity) to reduce race-induced overfills.
- Volume write path: writes are append-only; the code checks for duplicates and available space under the `lock`. If a write fails due to space, the Volume immediately sends a heartbeat indicating no-space for that vid.

## How to run the GUI Dashboard

The `./run` helper does not include the GUI by default. To start the JavaFX dashboard after building, you can run the main class directly with the project's classpath, for example:

```bash
./mvnw compile
CP=$(./mvnw -q dependency:build-classpath -Dmdep.outputFile=/dev/stdout)
java -cp build/classes:$CP deti.sd.moss.infra.ui.gui.MossDashboardApp
```

Alternatively open the project in an IDE and run `deti.sd.moss.infra.ui.gui.MossDashboardApp`.

## Known Limitations

- No replication or delete implemented in this base version (append-only storage, single copy per object).
- Single Manager (no leader election); Manager is a single point of coordination for this assignment.
