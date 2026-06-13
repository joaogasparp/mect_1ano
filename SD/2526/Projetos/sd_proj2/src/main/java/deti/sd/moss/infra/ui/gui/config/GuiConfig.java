package deti.sd.moss.infra.ui.gui.config;

// simple record to hold gui settings
public record GuiConfig(
    String managerUrl,
    String objectNodeUrl,
    String defaultBucket,
    int pollingIntervalMs
) {}
