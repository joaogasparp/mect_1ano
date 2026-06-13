package deti.sd.moss.core.manager;

import java.time.Instant;
import java.util.List;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import java.nio.file.Path;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.h2.mvstore.MVMap;
import org.h2.mvstore.MVStore;
import org.h2.mvstore.type.StringDataType;
import org.h2.mvstore.type.IntegerDataType;

import deti.sd.moss.core.common.ports.ServiceDiscovery;
import deti.sd.moss.core.common.model.VolumeInfo;

import deti.sd.moss.core.manager.ports.ManagerServiceProvider;
import deti.sd.moss.core.manager.model.*;

import deti.sd.moss.core.volume.model.AssignVolumeRequest;


public class ManagerServiceProviderImpl implements ManagerServiceProvider {
    private static final Logger logger = LoggerFactory.getLogger(ManagerServiceProviderImpl.class);

    private static final int MAX_VOLUME_SIZE = 1 << 25; // 32 MiB
    private static final int HEARTBEAT_INTERVAL = 5; // 5 seconds
    private static final int STALE_TIMEOUT_SECONDS = 20;

    private static final AtomicInteger FILE_ID_GENERATOR = new AtomicInteger(0);
    private static final AtomicInteger VOLUME_ID_GENERATOR = new AtomicInteger(0);

    // === Persistance ===
    private final MVStore store;
    private final MVMap<String, Integer> ledger;

    // === Services ===
    private final ServiceDiscovery discovery;

    // ---
    private static final int STATUS_OK = 0x00;
    private static final int STATUS_ERROR = 0x01;
    private static final int STATUS_TIMESTAMP_ERROR = 0x02;

    private final ConcurrentHashMap<Integer, VolumeLocalInfo> localRegistry = new ConcurrentHashMap<>();
    private final ConcurrentHashMap<String, NodeHeartbeatInfo> nodeRegistry = new ConcurrentHashMap<>();
    private final ScheduledExecutorService cleanupExecutor;

    private static final class NodeHeartbeatInfo {
        private final String url;
        private final long timestamp;

        private NodeHeartbeatInfo(String url, long timestamp) {
            this.url = url;
            this.timestamp = timestamp;
        }

        private String url() {
            return url;
        }

        private long timestamp() {
            return timestamp;
        }
    }

    public ManagerServiceProviderImpl(String mdir, ServiceDiscovery discovery) {
        this.discovery = discovery;
        this.cleanupExecutor = Executors.newSingleThreadScheduledExecutor();

        Path base = Path.of(mdir);
        Path data = base.resolve("manager.db");

        // Book keeping
        store = new MVStore.Builder()
                .fileName(data.toString())
                .open();

        ledger = store.openMap("ledger",
                new MVMap.Builder<String, Integer>()
                        .keyType(StringDataType.INSTANCE)
                        .valueType(IntegerDataType.INSTANCE));

        loadLedgerState();

        startStaleNodeCleanup();
        Runtime.getRuntime().addShutdownHook(new Thread(() -> cleanupExecutor.shutdownNow()));

    }

    private void loadLedgerState() {
        Integer lastFid = ledger.get("fid");
        if (lastFid != null) {
            FILE_ID_GENERATOR.set(lastFid);
        }

        Integer lastVid = ledger.get("vid");
        if (lastVid != null) {
            VOLUME_ID_GENERATOR.set(lastVid);
        }
    }

    private void startStaleNodeCleanup() {
        cleanupExecutor.scheduleAtFixedRate(
                this::removeStaleNodes,
                HEARTBEAT_INTERVAL,
                HEARTBEAT_INTERVAL,
                TimeUnit.SECONDS);
    }

    private void removeStaleNodes() {
        // Keep stale entries in the registries so state() can still report them.
        // Active operations filter by freshness instead of deleting history.
    }

    private List<VolumeLocalInfo> getActiveVolumes() {
        long cutoff = Instant.now().minusSeconds(STALE_TIMEOUT_SECONDS).toEpochMilli();
        return localRegistry.values().stream()
                .filter(info -> info.timestamp() >= cutoff)
                .toList();
    }

    private List<NodeHeartbeatInfo> getActiveNodes() {
        long cutoff = Instant.now().minusSeconds(STALE_TIMEOUT_SECONDS).toEpochMilli();
        return nodeRegistry.values().stream()
                .filter(info -> info.timestamp() >= cutoff)
                .toList();
    }

    @Override
    public AssignReply onAssign(AssignRequest request) {

        List<VolumeLocalInfo> activeVolumes = getActiveVolumes();
        List<NodeHeartbeatInfo> activeNodes = getActiveNodes();
        // Obs: Não é necessário (activeVolumes != Volumes)
        // if (activeVolumes.size() < 1) {
        // logger.error("No available volumes");
        // return new AssignReply("", "", 0, List.of());
        // }
        logger.info("Creating new assign");

        // Pick the best existing volume by available size (greedy)
        VolumeLocalInfo bestVolume = activeVolumes.stream()
                .max(Comparator.comparingInt(info -> info.vinfo().availableSize()))
                .orElse(null);

        if (bestVolume != null) {
            // conservative capacity estimation (rule 3.3): 
            // only use volumes that have more than 10% free space
            if (bestVolume.vinfo().availableSize() > MAX_VOLUME_SIZE * 0.10) {
                int vid = bestVolume.vinfo().vid();
                int fid = FILE_ID_GENERATOR.incrementAndGet();

                ledger.put("fid", fid);
                store.commit();
                String ticket = createTicket(vid, fid);
                logger.info("Assigning to {}", bestVolume.url());
                return new AssignReply(ticket, bestVolume.url(), 0, List.of());
            }
        }

        // No suitable existing volume -> request new volume on the node
        // with the least number of volumes (balanced allocation).
        // Build map url -> count
        if (activeNodes.isEmpty()) {
            logger.info("Failed to assign: no active volume nodes");
            return new AssignReply("", "", 0, List.of());
        }

        var countByUrl = new java.util.HashMap<String, Integer>();
        for (VolumeLocalInfo info : activeVolumes) {
            countByUrl.put(info.url(), countByUrl.getOrDefault(info.url(), 0) + 1);
        }
        logger.info("Volume count by URL: {}", countByUrl);

        // find node with minimum count
        String targetNode = activeNodes.stream()
                .min(Comparator.comparingInt(info -> countByUrl.getOrDefault(info.url(), 0)))
                .map(NodeHeartbeatInfo::url)
                .orElse(null);

        if (targetNode == null) {
            logger.info("Failed to assign");
            return new AssignReply("", "", 0, List.of());
        }

        int newVid = VOLUME_ID_GENERATOR.incrementAndGet();

        try {
            var volumeClient = discovery.getVolume(targetNode);
            var assignVolReply = volumeClient.assignVolume(new AssignVolumeRequest(newVid, List.of()));
            if (assignVolReply.status() != STATUS_OK) {
                logger.error("Failed to assign new volume on node {}", targetNode);
                return new AssignReply("", "", 0, List.of());
            }

            // persist vid and fid
            int fid = FILE_ID_GENERATOR.incrementAndGet();
            ledger.put("vid", newVid);
            ledger.put("fid", fid);
            store.commit();

            String ticket = createTicket(newVid, fid);
            logger.error("Successfully assigned new volume on node {}", targetNode);
            return new AssignReply(ticket, targetNode, 0, List.of());

        } catch (Exception e) {
            logger.error("Failed to assign new volume on node {}", targetNode, e);
            return new AssignReply("", "", 0, List.of());
        }
    }

    @Override
    public LookupReply onLookup(LookupRequest request) {

        String ticket = request.ticket();
        ParsedTicket parsedTicket = parseTicket(ticket);
        int volumeId = parsedTicket.volume();

        if (!localRegistry.containsKey(volumeId)) {
            // Invalid id
            return new LookupReply("");
        }

        String url = localRegistry.get(volumeId).url();

        return new LookupReply(url);
    }

    @Override
    public VolumeBeatReply onHeartbeat(VolumeBeatRequest request) {
        // TODO:
        // Se receber um heartbeat de um volume sem vid, criar um (sem isso vai dar
        // problemas no onAssign pois não vai ter nenhum nó ativo, assim garante que
        // todos os volumes a enviarem heartbeat estão ativos)
        // ter em atenção que o vinfo pode estar vazio
        try {
            long timestamp = Instant.now().toEpochMilli();

            nodeRegistry.put(request.url(), new NodeHeartbeatInfo(request.url(), timestamp));

            List<VolumeInfo> vinfo = request.vinfo();
            for (VolumeInfo info : vinfo) {
                int vid = info.vid();
                if (localRegistry.containsKey(vid) && (localRegistry.get(vid).timestamp() > timestamp)) {
                    return new VolumeBeatReply(STATUS_TIMESTAMP_ERROR);
                }
                localRegistry.put(vid, new VolumeLocalInfo(request.url(), request.count(), info, timestamp));
            }

            removeStaleNodes();

        } catch (Exception e) {
            return new VolumeBeatReply(STATUS_ERROR);
        }
        logger.info("Heartbeat received from {}", request.url());
        return new VolumeBeatReply(STATUS_OK);
    }

    @Override
    public StateReply onState() {
        List<StateReply.VolumeState> volumeStates = new ArrayList<>();

        for (VolumeLocalInfo volumeInfo : localRegistry.values()) {
            StateReply.VolumeState state = new StateReply.VolumeState(
                    volumeInfo.url(),
                    volumeInfo.timestamp(),
                    volumeInfo.count(),
                    List.of(volumeInfo.vinfo()));
            volumeStates.add(state);
        }

        return new StateReply(volumeStates);
    }

    public String createTicket(int volume, int id) {
        int randomValue = ThreadLocalRandom.current().nextInt();
        String cookie = String.format("%08x", randomValue);
        String ticket = String.valueOf(volume) + ":" + String.valueOf(id) + cookie;
        logger.info("Created ticket {}", ticket);
        return ticket;
    }

    public record ParsedTicket(int volume, int id, String cookie) {
    }

    public ParsedTicket parseTicket(String ticket) {
        int colonIndex = ticket.indexOf(':');
        // Volume
        int volume = Integer.parseInt(ticket.substring(0, colonIndex));

        // Cookie
        String cookie = ticket.substring(ticket.length() - 8);

        // ID
        int id = Integer.parseInt(ticket.substring(colonIndex + 1, ticket.length() - 8));

        return new ParsedTicket(volume, id, cookie);
    }
}
