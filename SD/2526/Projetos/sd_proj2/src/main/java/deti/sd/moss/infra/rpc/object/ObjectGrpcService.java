package deti.sd.moss.infra.rpc.object;

import io.grpc.stub.StreamObserver;

import deti.sd.moss.infra.rpc.object.ObjectGrpc;
import deti.sd.moss.infra.rpc.object.ProtoPutRequest;
import deti.sd.moss.infra.rpc.object.ProtoPutReply;
import deti.sd.moss.infra.rpc.object.ProtoGetRequest;
import deti.sd.moss.infra.rpc.object.ProtoGetReply;
import deti.sd.moss.infra.rpc.object.ProtoListRequest;
import deti.sd.moss.infra.rpc.object.ProtoListReply;
import deti.sd.moss.infra.rpc.object.ProtoObjectInfo;

import deti.sd.moss.core.object.ports.ObjectServiceProvider;
import deti.sd.moss.core.object.model.*;

public final class ObjectGrpcService extends ObjectGrpc.ObjectImplBase {

    private final ObjectServiceProvider manager;

    public ObjectGrpcService(ObjectServiceProvider manager){
        this.manager = manager;
    }

    @Override
    public void put(ProtoPutRequest protoRequest, StreamObserver<ProtoPutReply> responseObserver) {
        // 1. Map Protobuf -> Domain Model
        var request = new PutRequest(
            protoRequest.getBucket(),
            protoRequest.getPath(),
            protoRequest.getData().toByteArray()
        );

        // 2. Call the shared logic
        var response = manager.onPut(request);

        // 3. Map Domain Model -> Protobuf
        var reply = ProtoPutReply.newBuilder()
            .setStatus(response.status())
            .build();

        // 4. Send gRPC response
        responseObserver.onNext(reply);
        responseObserver.onCompleted();

    }

    @Override
    public void get(ProtoGetRequest protoRequest, StreamObserver<ProtoGetReply> responseObserver) {
        var request = new deti.sd.moss.core.object.model.GetRequest(
            protoRequest.getBucket(),
            protoRequest.getPath());

        var response = manager.onGet(request);

        var replyBuilder = ProtoGetReply.newBuilder()
            .setStatus(response.status());

        if (response.status() == 0) {
            replyBuilder.setData(com.google.protobuf.ByteString.copyFrom(response.data()));
        }

        responseObserver.onNext(replyBuilder.build());
        responseObserver.onCompleted();
    }

    @Override
    public void list(ProtoListRequest protoRequest, StreamObserver<ProtoListReply> responseObserver) {
        var request = new deti.sd.moss.core.object.model.ListRequest(protoRequest.getBucket());

        var response = manager.onList(request);

        var builder = ProtoListReply.newBuilder().setStatus(response.status());

        for (var bucket : response.bucketInfo()) {
            var info = ProtoObjectInfo.newBuilder()
                .setBucket(bucket.get(0))
                .setPath(bucket.get(1))
                .setSize(Integer.parseInt(bucket.get(2)))
                .setTimestamp(Long.parseLong(bucket.get(3)))
                .build();
            builder.addObjects(info);
        }

        responseObserver.onNext(builder.build());
        responseObserver.onCompleted();
    }

}
