package deti.sd.moss.infra.rpc.manager;

import io.grpc.stub.StreamObserver;

import deti.sd.moss.infra.rpc.manager.ManagerGrpc;
import deti.sd.moss.infra.rpc.manager.ProtoAssignRequest;
import deti.sd.moss.infra.rpc.manager.ProtoAssignReply;
import deti.sd.moss.infra.rpc.manager.ProtoLookupRequest;
import deti.sd.moss.infra.rpc.manager.ProtoLookupReply;
import deti.sd.moss.infra.rpc.manager.ProtoVolumeBeatRequest;
import deti.sd.moss.infra.rpc.manager.ProtoVolumeBeatReply;
import deti.sd.moss.infra.rpc.manager.ProtoStateReply;

import com.google.protobuf.Empty;

import deti.sd.moss.core.manager.ports.ManagerServiceProvider;
import deti.sd.moss.core.manager.model.*;

import deti.sd.moss.core.common.model.VolumeInfo;

public final class ManagerGrpcService extends ManagerGrpc.ManagerImplBase {

    private final ManagerServiceProvider manager;

    public ManagerGrpcService(ManagerServiceProvider manager){
        this.manager = manager;
    }

    @Override
    public void assign(ProtoAssignRequest protoRequest, StreamObserver<ProtoAssignReply> responseObserver) {
        // 1. Map Protobuf -> Domain Model
        var request = new AssignRequest(protoRequest.getReplicas());

        // 2. Call the shared logic
        var response = manager.onAssign(request);

        // 3. Map Domain Model -> Protobuf
        var reply = ProtoAssignReply.newBuilder()
            .setTicket(response.ticket())
            .setVolumeUrl(response.volumeUrl())
            .setCount(response.count())
            .addAllUrl(response.url() )
            .build();

        // 4. Send gRPC response
        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }

    @Override
    public void lookup(ProtoLookupRequest protoRequest, StreamObserver<ProtoLookupReply> responseObserver) {
        // 1. Map Protobuf -> Domain Model
        var request = new LookupRequest(protoRequest.getTicket());

        // 2. Call the shared logic
        var response = manager.onLookup(request);

        // 3. Map Domain Model -> Protobuf
        var reply = ProtoLookupReply.newBuilder()
            .setUrl(response.url())
            .build();

        // 4. Send gRPC response
        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }


    @Override
    public void heartbeat(ProtoVolumeBeatRequest protoRequest,
                          StreamObserver<ProtoVolumeBeatReply> responseObserver) {

        // 1. Map Protobuf -> Domain Model
        var request = new VolumeBeatRequest(
            protoRequest.getUrl(),
            protoRequest.getCount(),
            protoRequest.getVinfoList().stream()
                .map(item -> new VolumeInfo(
                    item.getVid(),
                    item.getFileCount(),
                    item.getAvailableSize(),
                    item.getStatus())).toList()
        );

        // 2. Call the shared logic
        var response = manager.onHeartbeat(request);

        // 3. Map Domain Model -> Protobuf
        var reply = ProtoVolumeBeatReply.newBuilder()
            .setStatus(response.status())
            .build();

        // 4. Send gRPC response
        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }

    @Override
    public void state(Empty protoRequest, StreamObserver<ProtoStateReply> responseObserver) {
        var response = manager.onState();

        var protoNodes = response.nodes().stream()
            .map(node -> ProtoStateReply.ProtoVolumeState.newBuilder()
                .setUrl(node.url())
                .setLastSeenEpochMs(node.lastSeenEpochMs())
                .setCount(node.count())
                .addAllVinfo(node.vinfo().stream()
                    .map(info -> ProtoVolumeBeatRequest.ProtoVolumeInfo.newBuilder()
                        .setVid(info.vid())
                        .setFileCount(info.fileCount())
                        .setAvailableSize(info.availableSize())
                        .setStatus(info.status())
                        .build())
                    .toList())
                .build())
            .toList();

        var reply = ProtoStateReply.newBuilder()
            .addAllNodes(protoNodes)
            .build();

        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }


}
