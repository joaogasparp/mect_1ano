package deti.sd.moss.core.object.ports;

import deti.sd.moss.core.object.model.*;

public interface ObjectService {
    public PutReply put(PutRequest request);

    // TODO: Get interface
    public GetReply get(GetRequest request);

    public ListReply list(ListRequest request);
}
