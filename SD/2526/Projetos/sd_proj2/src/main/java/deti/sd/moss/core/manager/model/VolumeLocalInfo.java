package deti.sd.moss.core.manager.model;

import java.util.List;

import deti.sd.moss.core.common.model.VolumeInfo;

public record VolumeLocalInfo(
        String url,
        int count,
        VolumeInfo vinfo,
        Long timestamp) {
}