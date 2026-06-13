package deti.sd.moss.infra.ui.gui.backend;

import deti.sd.moss.infra.rpc.discovery.GrpcServiceDiscovery;
import deti.sd.moss.core.manager.ports.ManagerService;
import deti.sd.moss.core.object.ports.ObjectService;
import deti.sd.moss.core.manager.model.StateReply;
import deti.sd.moss.core.object.model.*;

import java.util.ArrayList;
import java.util.List;

// backend adapter to talk to manager and object nodes
public class DashboardBackend {
    private final GrpcServiceDiscovery discovery;
    private final String managerUrl;
    private final String objectUrl;

    public DashboardBackend(String managerUrl, String objectUrl) {
        this.discovery = new GrpcServiceDiscovery();
        this.managerUrl = managerUrl;
        this.objectUrl = objectUrl;
    }

    // fetch the cluster topology and volume health from manager
    public StateReply getClusterState() {
        ManagerService manager = discovery.getManager(managerUrl);
        return manager.state();
    }

    // fetch all objects in a bucket from the object node
    public List<ObjectEntryModel> getObjectInventory(String bucket) {
        ObjectService objectService = discovery.getObject(objectUrl);
        ListReply reply = objectService.list(new ListRequest(bucket));
        
        List<ObjectEntryModel> result = new ArrayList<>();
        if (reply.status() == 0 && reply.bucketInfo() != null) {
            for (ArrayList<String> info : reply.bucketInfo()) {
                if (info.size() >= 4) {
                    // default vid to -1 if the backend doesn't provide the ticket
                    int vid = -1;
                    if (info.size() >= 5) {
                        String ticket = info.get(4);
                        vid = Integer.parseInt(ticket.split(":")[0]);
                    }

                    result.add(new ObjectEntryModel(
                        info.get(0), // bucket
                        info.get(1), // path
                        Long.parseLong(info.get(2)), // size
                        Long.parseLong(info.get(3)), // timestamp
                        vid
                    ));
                }
            }
        }
        return result;
    }

    // trigger a put operation directly from gui
    public int putObject(String bucket, String path, byte[] data) {
        ObjectService objectService = discovery.getObject(objectUrl);
        return objectService.put(new PutRequest(bucket, path, data)).status();
    }

    // trigger a get operation
    public byte[] getObject(String bucket, String path) {
        ObjectService objectService = discovery.getObject(objectUrl);
        GetReply reply = objectService.get(new GetRequest(bucket, path));
        return (reply.status() == 0) ? reply.data() : null;
    }

    // simple model for table binding in javafx
    public record ObjectEntryModel(String bucket, String path, long size, long timestamp, int vid) {}
}
