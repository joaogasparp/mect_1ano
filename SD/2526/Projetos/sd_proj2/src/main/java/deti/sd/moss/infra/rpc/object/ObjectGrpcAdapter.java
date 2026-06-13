package deti.sd.moss.infra.rpc.object;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import io.grpc.ManagedChannel;

import deti.sd.moss.util.StubRunner;

import deti.sd.moss.infra.rpc.object.ObjectGrpc;
import deti.sd.moss.infra.rpc.object.ProtoPutRequest;
import deti.sd.moss.infra.rpc.object.ProtoPutReply;
import deti.sd.moss.infra.rpc.object.ProtoGetRequest;
import deti.sd.moss.infra.rpc.object.ProtoGetReply;
import deti.sd.moss.infra.rpc.object.ProtoListRequest;
import deti.sd.moss.infra.rpc.object.ProtoListReply;
import deti.sd.moss.infra.rpc.object.ProtoObjectInfo;

import deti.sd.moss.core.object.ports.ObjectService;
import deti.sd.moss.core.object.model.*;

public class ObjectGrpcAdapter implements ObjectService {
    private static final Logger logger = LoggerFactory.getLogger(ObjectGrpcAdapter.class);

    private final ObjectGrpc.ObjectBlockingStub stub;

    public ObjectGrpcAdapter(ManagedChannel channel) {
        this.stub = ObjectGrpc.newBlockingStub(channel);
    }

    @Override
    public PutReply put(PutRequest request){

        // 1. Map Domain Model -> Protobuf
        ProtoPutRequest protoRequest = ProtoPutRequest.newBuilder()
            .setBucket(request.bucket())
            .setPath(request.path())
            .setData(com.google.protobuf.ByteString.copyFrom(request.data()))
            .build();

        // 2. Make the call
        ProtoPutReply response = StubRunner.execute(() -> stub.put(protoRequest))
            .orFail(() -> {
                logger.error("gRPC conneection failed");
                return ProtoPutReply.newBuilder().setStatus(-1).build();
            });

        // 3. Map Protobuf -> Domain Model
        return new PutReply(
            response.getStatus()
        );
    }

    @Override
    public GetReply get(GetRequest request) {

        // 1. Map Domain Model -> Protobuf
        ProtoGetRequest protoRequest = ProtoGetRequest.newBuilder()
            .setBucket(request.bucket())
            .setPath(request.path())
            .build();

        // 2. Make the call
        ProtoGetReply response = StubRunner.execute(() -> stub.get(protoRequest))
            .orFail(() -> {
                logger.error("gRPC connection failed");
                return ProtoGetReply.getDefaultInstance();
            });

        // 3. Map Protobuf -> Domain Model
        return new GetReply(
            response.getStatus(),
            response.getData().toByteArray()
        );
    }

    @Override
    public ListReply list(ListRequest request) {

        // 1. Map Domain Model -> Protobuf
        ProtoListRequest protoRequest = ProtoListRequest.newBuilder()
            .setBucket(request.bucket())
            .build();

        // 2. Make the call
        ProtoListReply response = StubRunner.execute(() -> stub.list(protoRequest))
            .orFail(() -> {
                logger.error("gRPC connection failed");
                return ProtoListReply.getDefaultInstance();
            });

        // 3. Map Protobuf -> Domain Model
        java.util.ArrayList<java.util.ArrayList<String>> bucketInfo = new java.util.ArrayList<>();
        
        for (ProtoObjectInfo objInfo : response.getObjectsList()) {
            java.util.ArrayList<String> info = new java.util.ArrayList<>();
            info.add(objInfo.getBucket());
            info.add(objInfo.getPath());
            info.add(String.valueOf(objInfo.getSize()));
            info.add(String.valueOf(objInfo.getTimestamp()));
            bucketInfo.add(info);
        }

        return new ListReply(
            response.getStatus(),
            bucketInfo
        );
    }

}

