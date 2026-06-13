package deti.sd.moss.infra.rpc.volume;

import io.grpc.ManagedChannel;

import deti.sd.moss.util.StubRunner;

import deti.sd.moss.infra.rpc.volume.VolumeGrpc;
import deti.sd.moss.infra.rpc.volume.ProtoAssignVolumeRequest;
import deti.sd.moss.infra.rpc.volume.ProtoAssignVolumeReply;
import deti.sd.moss.infra.rpc.volume.ProtoWriteRequest;
import deti.sd.moss.infra.rpc.volume.ProtoWriteReply;
import deti.sd.moss.infra.rpc.volume.ProtoReadRequest;
import deti.sd.moss.infra.rpc.volume.ProtoReadReply;

import deti.sd.moss.core.volume.ports.VolumeService;
import deti.sd.moss.core.volume.model.*;

public class VolumeGrpcAdapter implements VolumeService {
    private final VolumeGrpc.VolumeBlockingStub stub;

    public VolumeGrpcAdapter(ManagedChannel channel) {
        this.stub = VolumeGrpc.newBlockingStub(channel);
    }

    @Override
    public AssignVolumeReply assignVolume(AssignVolumeRequest request){

        // 1. Map Domain Model -> Protobuf
        var protoRequest = ProtoAssignVolumeRequest.newBuilder()
            .setVid(request.vid())
            .addAllUrls(request.url())
            .build();

        // 2. Make the call
        var response = StubRunner.execute(() -> stub.assignVolume(protoRequest))
            .orFail(() -> {
                System.out.println("Failed to connect");
                return ProtoAssignVolumeReply.getDefaultInstance();
            });

        // 3. Map Protobuf -> Domain Model
        return new AssignVolumeReply(
            response.getStatus()
        );
    }

    @Override
    public WriteReply write(WriteRequest request) {

        // 1. Map Domain Model -> Protobuf
        var protoRequest = ProtoWriteRequest.newBuilder()
            .setVid(request.vid())
            .setFid(request.fid())
            .setCookie(request.cookie())
            .setData(com.google.protobuf.ByteString.copyFrom(request.data()))
            .build();

        // 2. Make the call
        var response = StubRunner.execute(() -> stub.write(protoRequest))
            .orFail(() -> {
                System.out.println("Failed to connect");
                return ProtoWriteReply.getDefaultInstance();
            });

        // 3. Map Protobuf -> Domain Model
        return new WriteReply(
            response.getStatus()
        );
    }

    @Override
    public ReadReply read(ReadRequest request) {

        // 1. Map Domain Model -> Protobuf
        var protoRequest = ProtoReadRequest.newBuilder()
            .setVid(request.vid())
            .setFid(request.fid())
            .setCookie(request.cookie())
            .build();

        // 2. Make the call
        var response = StubRunner.execute(() -> stub.read(protoRequest))
            .orFail(() -> {
                System.out.println("Failed to connect");
                return ProtoReadReply.getDefaultInstance();
            });

        // 3. Map Protobuf -> Domain Model
        return new ReadReply(
            response.getStatus(),
            response.getData().toByteArray()
        );
    }

}

