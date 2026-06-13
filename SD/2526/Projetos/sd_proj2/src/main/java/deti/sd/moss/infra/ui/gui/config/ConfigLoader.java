package deti.sd.moss.infra.ui.gui.config;

import org.yaml.snakeyaml.Yaml;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Map;

// utility to load the yaml configuration
public class ConfigLoader {
    public static GuiConfig load(String filename) {
        try {
            Yaml yaml = new Yaml();
            Path path = Path.of(filename);
            
            // if file doesn't exist, return defaults
            if (!Files.exists(path)) {
                return new GuiConfig("localhost:4081", "localhost:4281", "sd", 5000);
            }

            try (InputStream in = Files.newInputStream(path)) {
                Map<String, Object> data = yaml.load(in);
                return new GuiConfig(
                    (String) data.getOrDefault("manager_url", "localhost:4081"),
                    (String) data.getOrDefault("object_node_url", "localhost:4281"),
                    (String) data.getOrDefault("default_bucket", "sd"),
                    ((Number) data.getOrDefault("polling_interval_ms", 5000)).intValue()
                );
            }
        } catch (Exception e) {
            // fallback to defaults on error
            return new GuiConfig("localhost:4081", "localhost:4281", "sd", 5000);
        }
    }
}
