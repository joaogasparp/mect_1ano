package deti.sd.moss.core.manager.model;

import java.util.List;
import deti.sd.moss.core.common.model.VolumeInfo;

public record StateReply(List<VolumeState> nodes) {
    
    public record VolumeState(
        String url,
        long lastSeenEpochMs,
        int count,
        List<VolumeInfo> vinfo
    ) {}
}
