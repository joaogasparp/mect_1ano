package deti.sd.moss.core.object;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.file.Path;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.h2.mvstore.MVMap;
import org.h2.mvstore.MVStore;
import org.h2.mvstore.type.StringDataType;

import deti.sd.moss.core.common.ports.ServiceDiscovery;
import deti.sd.moss.core.manager.ports.ManagerService;
import deti.sd.moss.core.volume.ports.VolumeService;
import deti.sd.moss.core.object.ports.ObjectServiceProvider;

import deti.sd.moss.core.manager.model.*;
import deti.sd.moss.core.volume.model.*;
import deti.sd.moss.core.object.model.*;

public class ObjectServiceProviderImpl implements ObjectServiceProvider {
    private static final Logger logger = LoggerFactory.getLogger(ObjectServiceProviderImpl.class);

    // === Communications ===
    private final ServiceDiscovery discovery;
    private final ManagerService manager;

    private final MVStore db;
    private final MVMap<String, ObjectEntry> objects;
    private final ReadWriteLock objectsLock = new ReentrantReadWriteLock();

    public ObjectServiceProviderImpl(String managerUrl, String dbname, ServiceDiscovery discovery) {
        this.discovery = discovery;
        this.manager = discovery.getManager(managerUrl);

        Path base = Path.of(dbname);
        db = new MVStore.Builder().fileName(base.toString()).open();

        // open the map for object metadata
        objects = db.openMap("objects", new MVMap.Builder<String, ObjectEntry>()
                .keyType(StringDataType.INSTANCE)
                .valueType(new ObjectEntryDataType()));
    }

    @Override
    public GetReply onGet(GetRequest request) {
        // TODO: Implement
        // check the object metadata locally
        String key = request.bucket() + "/" + request.path();
        ObjectEntry entry;
        objectsLock.readLock().lock();
        try {
            entry = objects.get(key);
        } finally {
            objectsLock.readLock().unlock();
        }

        if (entry == null) {
            logger.error("Object not found: {}", key);
            return new GetReply(1, new byte[0]);
        }

        // ask the manager for the volume location
        // we take the first ticket
        String ticketStr = entry.ticket().get(0);
        var lookupReply = manager.lookup(new LookupRequest(ticketStr));

        if (lookupReply.url().isEmpty()) {
            logger.error("Manager could not resolve volume for ticket: {}", ticketStr);
            return new GetReply(1, new byte[0]);
        }

        // fetch the data from the volume node
        var ticket = parseTicket(ticketStr);
        var volume = discovery.getVolume(lookupReply.url());

        var readReply = volume.read(new ReadRequest(
                ticket.vid,
                ticket.fid,
                ticket.cookie));

        if (readReply.status() != 0) {
            logger.error("Volume node failed to read data");
            return new GetReply(1, new byte[0]);
        }

        logger.info("Retrieved object: {} from bucket: {}", request.path(), request.bucket());
        return new GetReply(0, readReply.data());
    }

    @Override
    public PutReply onPut(PutRequest request) {
        // TODO: Implement
        // verify if file size < 4mb
        if (request.data().length > 4 * 1024 * 1024) {
            logger.error("Object size exceeds 4MB limit");
            // logger.error("File size exceeds 4MB limit: {} bytes",
            // request.getData().length);
            return new PutReply(-1);
        }

        // ask manager for a volume to store the object
        var assignReply = manager.assign(new AssignRequest(1));
        if (assignReply.ticket().isEmpty()) {
            logger.error("No available volume to store the object");
            return new PutReply(-1);
        }

        // parse ticket and send data to volume
        var ticket = parseTicket(assignReply.ticket());
        var volume = discovery.getVolume(assignReply.volumeUrl());

        // write to volume node
        var writeReply = volume.write(new WriteRequest(
                ticket.vid,
                ticket.fid,
                ticket.cookie,
                request.data()));

        if (writeReply.status() != 0) {
            logger.error("Failed to write data to volume node");
            return new PutReply(-1);
        }

        // record metadata locally
        String key = request.bucket() + "/" + request.path();
        var entry = new ObjectEntry(
                List.of(assignReply.ticket()),
                Instant.now(), // timestamp
                request.data().length);
        objectsLock.writeLock().lock();
        try {
            objects.put(key, entry);
            db.commit();
        } finally {
            objectsLock.writeLock().unlock();
        }

        logger.info("Stored object: {} in bucket: {}", request.path(), request.bucket());
        return new PutReply(0);
    }

    private record ParsedTicket(int vid, int fid, int cookie) {
    }

    @Override
    public ListReply onList(ListRequest request) {
        ArrayList<ArrayList<String>> bucketInfo = new ArrayList<>();
        objectsLock.readLock().lock();
        try {
            objects.entrySet().stream()
                .filter(entry -> entry.getKey().startsWith(request.bucket() + "/"))
                .forEach(entry -> {
                    String key = entry.getKey();
                    ObjectEntry objEntry = entry.getValue();
                    String path = key.substring(request.bucket().length() + 1); // Remove "bucket/"

                    ArrayList<String> info = new ArrayList<>();
                    info.add(request.bucket());
                    info.add(path);
                    info.add(String.valueOf(objEntry.size()));
                    info.add(String.valueOf(objEntry.timestamp().toEpochMilli()));

                    bucketInfo.add(info);
                });
        } finally {
            objectsLock.readLock().unlock();
        }
        
        logger.info("Listed {} objects in bucket: {}", bucketInfo.size(), request.bucket());
        return new ListReply(0, bucketInfo);
    }

    private ParsedTicket parseTicket(String ticket) {
        int colonIndex = ticket.indexOf(':');
        int vid = Integer.parseInt(ticket.substring(0, colonIndex));
        String cookieStr = ticket.substring(ticket.length() - 8);
        int fid = Integer.parseInt(ticket.substring(colonIndex + 1, ticket.length() - 8));
        int cookie = (int) Long.parseUnsignedLong(cookieStr, 16);
        return new ParsedTicket(vid, fid, cookie);
    }
}
