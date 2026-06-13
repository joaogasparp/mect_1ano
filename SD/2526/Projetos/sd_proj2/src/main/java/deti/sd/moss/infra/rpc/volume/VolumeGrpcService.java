package deti.sd.moss.infra.rpc.volume;

import io.grpc.stub.StreamObserver;

import deti.sd.moss.infra.rpc.volume.VolumeGrpc;
import deti.sd.moss.infra.rpc.volume.ProtoAssignVolumeRequest;
import deti.sd.moss.infra.rpc.volume.ProtoAssignVolumeReply;
import deti.sd.moss.infra.rpc.volume.ProtoWriteRequest;
import deti.sd.moss.infra.rpc.volume.ProtoWriteReply;
import deti.sd.moss.infra.rpc.volume.ProtoReadRequest;
import deti.sd.moss.infra.rpc.volume.ProtoReadReply;

import deti.sd.moss.core.volume.ports.VolumeServiceProvider;
import deti.sd.moss.core.volume.model.*;

public final class VolumeGrpcService extends VolumeGrpc.VolumeImplBase {

    private final VolumeServiceProvider provider;

    public VolumeGrpcService(VolumeServiceProvider provider){
        this.provider = provider;
    }

    @Override
    public void assignVolume(ProtoAssignVolumeRequest protoRequest,
                    StreamObserver<ProtoAssignVolumeReply> responseObserver) {

        // 1. Map Protobuf -> Domain Model
        var request = new AssignVolumeRequest(
            protoRequest.getVid(),
            protoRequest.getUrlsList()
        );

        // 2. Call the shared logic
        var response = provider.onAssignVolume(request);

        // 3. Map Domain Model -> Protobuf
        var reply = ProtoAssignVolumeReply.newBuilder()
            .setStatus(response.status())
            .build();

        // 4. Send gRPC response
        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }

    @Override
    public void write(ProtoWriteRequest protoRequest,
                    StreamObserver<ProtoWriteReply> responseObserver) {

        // 1. Map Protobuf -> Domain Model
        var request = new WriteRequest(
            protoRequest.getVid(),
            protoRequest.getFid(),
            protoRequest.getCookie(),
            protoRequest.getData().toByteArray()
        );

        // 2. Call the shared logic
        var response = provider.onWrite(request);

        // 3. Map Domain Model -> Protobuf
        var reply = ProtoWriteReply.newBuilder()
            .setStatus(response.status())
            .build();

        // 4. Send gRPC response
        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }

    @Override
    public void read(ProtoReadRequest protoRequest,
                    StreamObserver<ProtoReadReply> responseObserver) {

        // 1. Map Protobuf -> Domain Model
        var request = new ReadRequest(
            protoRequest.getVid(),
            protoRequest.getFid(),
            protoRequest.getCookie()
        );

        // 2. Call the shared logic
        var response = provider.onRead(request);

        // 3. Map Domain Model -> Protobuf
        var reply = ProtoReadReply.newBuilder()
            .setStatus(response.status())
            .setData(com.google.protobuf.ByteString.copyFrom(response.data()))
            .build();

        // 4. Send gRPC response
        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }

}
