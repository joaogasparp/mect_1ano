package deti.sd.moss.infra.ui.gui;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.stage.Stage;
import deti.sd.moss.infra.ui.gui.config.ConfigLoader;
import deti.sd.moss.infra.ui.gui.config.GuiConfig;

// main entry point for the dashboard
public class MossDashboardApp extends Application {
    @Override
    public void start(Stage primaryStage) {
        // load configuration from yaml
        GuiConfig config = ConfigLoader.load("moss-gui.yaml");

        // create the main view
        DashboardView root = new DashboardView(config);

        // use screen bounds for initial size to prevent resizing issues on linux
        javafx.stage.Screen screen = javafx.stage.Screen.getPrimary();
        javafx.geometry.Rectangle2D bounds = screen.getVisualBounds();
        Scene scene = new Scene(root, bounds.getWidth(), bounds.getHeight());

        primaryStage.setTitle("MOSS Monitoring Dashboard");
        primaryStage.setScene(scene);
        primaryStage.setMaximized(true);

        // stop background threads on close
        primaryStage.setOnCloseRequest(e -> root.stopPolling());

        primaryStage.show();
    }

    public static void main(String[] args) {
        launch(args);
    }
}
