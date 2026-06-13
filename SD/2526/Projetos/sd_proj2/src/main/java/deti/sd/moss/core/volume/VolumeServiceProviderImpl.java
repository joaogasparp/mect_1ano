package deti.sd.moss.core.volume;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import deti.sd.moss.core.common.ports.ServiceDiscovery;
import deti.sd.moss.core.manager.ports.ManagerService;
import deti.sd.moss.core.volume.ports.VolumeServiceProvider;

import deti.sd.moss.core.volume.model.*;

import deti.sd.moss.core.common.model.VolumeInfo;
import deti.sd.moss.core.manager.model.VolumeBeatRequest;
import deti.sd.moss.core.manager.model.VolumeBeatReply;

import deti.sd.moss.util.NetworkUtils;

public class VolumeServiceProviderImpl implements VolumeServiceProvider {
    private static final Logger logger = LoggerFactory.getLogger(VolumeServiceProviderImpl.class);

    private static final int MAX_VOLUME_SIZE = 2 << 24; // 32MB
    private static final int MAX_NUMBER_VOLUMES = 5; // 160MB per volume node
    private static final int HEARTBEAT_INTERVAL_SECONDS = 5;

    private static final int STATUS_OK = 0;
    private static final int STATUS_ERROR = 1;
    private static final int STATUS_NOT_FOUND = 2;
    private static final int STATUS_COOKIE_MISMATCH = 3;
    private static final int STATUS_DUPLICATE_FILE = 4;
    private static final int STATUS_NO_SPACE = 5;
    private static final int STATUS_VOLUME_LIMIT = 6;

    private final ServiceDiscovery discovery;
    private final ManagerService manager;

    private final String basedir;
    private final String selfUrl;
    private final ScheduledExecutorService heartbeatExecutor;

    private final Lock lock = new ReentrantLock();
    private final Map<Integer, VolumeState> volumes = new ConcurrentHashMap<>();

    private record FileEntry(int cookie, long offset, int size) {
    }

    private static final class VolumeState {
        final int vid;
        final Path idxPath;
        final Path dataPath;
        final Map<Integer, FileEntry> files;

        VolumeState(int vid, Path idxPath, Path dataPath, Map<Integer, FileEntry> files) {
            this.vid = vid;
            this.idxPath = idxPath;
            this.dataPath = dataPath;
            this.files = files;
        }
    }

    public VolumeServiceProviderImpl(int port, String managerUrl, String dir, ServiceDiscovery discovery) {
        this.discovery = discovery;
        this.manager = discovery.getManager(managerUrl);
        this.basedir = dir;
        this.heartbeatExecutor = Executors.newSingleThreadScheduledExecutor();

        this.selfUrl = String.format("%s:%d", NetworkUtils.getRealIPv4(), port);

        loadExistingVolumes();
        startHeartbeat();

        Runtime.getRuntime().addShutdownHook(new Thread(() -> heartbeatExecutor.shutdownNow()));
    }

    private void startHeartbeat() {
        heartbeatExecutor.scheduleAtFixedRate(
                this::sendHeartbeat,
                0,
                HEARTBEAT_INTERVAL_SECONDS,
                TimeUnit.SECONDS);
    }

    private void sendHeartbeat() {
        sendHeartbeat(-1);
    }

    private void sendHeartbeat(int forceNoSpaceVid) {
        var snapshot = new ArrayList<VolumeInfo>();

        lock.lock();
        try {
            for (VolumeState state : volumes.values()) {
                int availableSize = getAvailableSize(state);
                int status = (state.vid == forceNoSpaceVid || availableSize <= 0)
                        ? STATUS_NO_SPACE
                        : STATUS_OK;

                snapshot.add(new VolumeInfo(
                        state.vid,
                        state.files.size(),
                        availableSize,
                        status));
            }
        } finally {
            lock.unlock();
        }

        try {
            VolumeBeatReply reply = manager.heartbeat(new VolumeBeatRequest(
                    selfUrl,
                    snapshot.size(),
                    snapshot));

            if (reply.status() != STATUS_OK) {
                logger.warn("manager heartbeat returned status {}", reply.status());
            }
        } catch (Exception e) {
            logger.debug("heartbeat to manager failed", e);
        }
    }

    private int getAvailableSize(VolumeState state) {
        try {
            long used = Files.size(state.dataPath);
            long remaining = MAX_VOLUME_SIZE - used;
            return (int) Math.max(remaining, 0);
        } catch (IOException e) {
            logger.warn("failed to compute available size for vid {}", state.vid, e);
            return 0;
        }
    }

    private void loadExistingVolumes() {
        try {
            Path basePath = Path.of(basedir);
            Files.createDirectories(basePath);

            try (var stream = Files.list(basePath)) {
                stream.filter(path -> path.getFileName().toString().endsWith(".idx"))
                        .forEach(this::loadSingleVolume);
            }
        } catch (IOException e) {
            logger.error("Failed to load existing volumes from {}", basedir, e);
        }
    }

    private void loadSingleVolume(Path idxPath) {
        String fileName = idxPath.getFileName().toString();
        int dot = fileName.indexOf('.');
        if (dot <= 0) {
            return;
        }

        int vid;
        try {
            vid = Integer.parseInt(fileName.substring(0, dot));
        } catch (NumberFormatException e) {
            return;
        }

        Path dataPath = idxPath.getParent().resolve(vid + ".data");
        if (!Files.exists(dataPath)) {
            return;
        }

        Map<Integer, FileEntry> files = new ConcurrentHashMap<>();
        try {
            for (String line : Files.readAllLines(idxPath, StandardCharsets.UTF_8)) {
                if (line.isBlank()) {
                    continue;
                }

                String[] parts = line.split(",");
                if (parts.length != 4) {
                    continue;
                }

                int fid = Integer.parseInt(parts[0]);
                int cookie = Integer.parseInt(parts[1]);
                long offset = Long.parseLong(parts[2]);
                int size = Integer.parseInt(parts[3]);
                files.put(fid, new FileEntry(cookie, offset, size));
            }
        } catch (Exception e) {
            logger.warn("Skipping unreadable index file: {}", idxPath, e);
            return;
        }

        volumes.put(vid, new VolumeState(vid, idxPath, dataPath, files));
        logger.info("Loaded volume {} with {} files", vid, files.size());
    }

    @Override
    public AssignVolumeReply onAssignVolume(AssignVolumeRequest request) {
        lock.lock();
        try {
            if (volumes.containsKey(request.vid())) {
                return new AssignVolumeReply(STATUS_OK);
            }

            if (volumes.size() >= MAX_NUMBER_VOLUMES) {
                return new AssignVolumeReply(STATUS_VOLUME_LIMIT);
            }

            Path basePath = Path.of(basedir);
            Files.createDirectories(basePath);

            Path idxPath = basePath.resolve(request.vid() + ".idx");
            Path dataPath = basePath.resolve(request.vid() + ".data");

            if (!Files.exists(idxPath)) {
                Files.createFile(idxPath);
            }
            if (!Files.exists(dataPath)) {
                Files.createFile(dataPath);
            }

            volumes.put(request.vid(), new VolumeState(
                    request.vid(),
                    idxPath,
                    dataPath,
                    new ConcurrentHashMap<>()));

            sendHeartbeat();
            return new AssignVolumeReply(STATUS_OK);
        } catch (IOException e) {
            logger.error("Failed to assign volume {}", request.vid(), e);
            return new AssignVolumeReply(STATUS_ERROR);
        } finally {
            lock.unlock();
        }
    }

    @Override
    public WriteReply onWrite(WriteRequest request) {
        int noSpaceVid = -1;
        lock.lock();
        try {
            VolumeState state = volumes.get(request.vid());
            if (state == null) {
                return new WriteReply(STATUS_NOT_FOUND);
            }

            if (state.files.containsKey(request.fid())) {
                return new WriteReply(STATUS_DUPLICATE_FILE);
            }

            long usedBytes = Files.size(state.dataPath);
            if (usedBytes + request.data().length > MAX_VOLUME_SIZE) {
                noSpaceVid = request.vid();
                return new WriteReply(STATUS_NO_SPACE);
            }

            long offset = usedBytes;
            Files.write(
                    state.dataPath,
                    request.data(),
                    StandardOpenOption.APPEND);

            String entry = String.format(
                    "%d,%d,%d,%d%n",
                    request.fid(),
                    request.cookie(),
                    offset,
                    request.data().length);
            Files.writeString(
                    state.idxPath,
                    entry,
                    StandardCharsets.UTF_8,
                    StandardOpenOption.APPEND);

            state.files.put(
                    request.fid(),
                    new FileEntry(request.cookie(), offset, request.data().length));
            logger.info("Wrote file {} on {}", request.fid(), request.vid());

            return new WriteReply(STATUS_OK);
        } catch (IOException e) {
            logger.error("Failed writing fid {} on vid {}", request.fid(), request.vid(), e);
            return new WriteReply(STATUS_ERROR);
        } finally {
            lock.unlock();
            if (noSpaceVid >= 0) {
                sendHeartbeat(noSpaceVid);
            }
        }
    }

    @Override
    public ReadReply onRead(ReadRequest request) {
        lock.lock();
        try {
            VolumeState state = volumes.get(request.vid());
            if (state == null) {
                return new ReadReply(STATUS_NOT_FOUND, new byte[0]);
            }

            FileEntry entry = state.files.get(request.fid());
            if (entry == null) {
                return new ReadReply(STATUS_NOT_FOUND, new byte[0]);
            }

            if (entry.cookie() != request.cookie()) {
                return new ReadReply(STATUS_COOKIE_MISMATCH, new byte[0]);
            }

            byte[] allData = Files.readAllBytes(state.dataPath);
            long end = entry.offset() + entry.size();
            if (entry.offset() < 0 || end > allData.length) {
                return new ReadReply(STATUS_ERROR, new byte[0]);
            }

            byte[] data = new byte[entry.size()];
            System.arraycopy(allData, (int) entry.offset(), data, 0, entry.size());

            logger.info("Read file {} from {}", request.fid(), request.vid());

            return new ReadReply(STATUS_OK, data);
        } catch (IOException e) {
            logger.error("Failed reading fid {} on vid {}", request.fid(), request.vid(), e);
            return new ReadReply(STATUS_ERROR, new byte[0]);
        } finally {
            lock.unlock();
        }
    }
}
