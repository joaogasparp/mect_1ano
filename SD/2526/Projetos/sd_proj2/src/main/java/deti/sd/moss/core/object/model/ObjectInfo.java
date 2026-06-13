package deti.sd.moss.core.object.model;

public record ObjectInfo(
    String bucket,
    String path,
    int size,
    long timestamp
) {}
