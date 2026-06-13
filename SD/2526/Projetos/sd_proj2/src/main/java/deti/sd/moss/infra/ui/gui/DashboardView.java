package deti.sd.moss.infra.ui.gui;

import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.stage.FileChooser;
import deti.sd.moss.infra.ui.gui.config.GuiConfig;
import deti.sd.moss.infra.ui.gui.backend.DashboardBackend;
import deti.sd.moss.core.manager.model.StateReply;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

// main view of the dashboard
public class DashboardView extends BorderPane {
    private static final long MAX_FILE_SIZE_BYTES = 4L * 1024 * 1024;
    private static final Path COMPOSE_FILE = Path.of("docker", "compose.yml");

    private final GuiConfig config;
    private final DashboardBackend backend;
    private final ScheduledExecutorService scheduler;
    private final ExecutorService actionExecutor;

    // table for cluster topology
    private TableView<VolumeNodeModel> volumeTable;
    private final ObservableList<VolumeNodeModel> volumeList = FXCollections.observableArrayList();

    // table for object inventory
    private TableView<DashboardBackend.ObjectEntryModel> objectTable;
    private final ObservableList<DashboardBackend.ObjectEntryModel> objectList = FXCollections.observableArrayList();
    private TextField bucketField;
    private TextField searchField;
    private TextArea activityLog;

    public DashboardView(GuiConfig config) {
        this.config = config;
        this.backend = new DashboardBackend(config.managerUrl(), config.objectNodeUrl());
        this.scheduler = Executors.newSingleThreadScheduledExecutor();
        this.actionExecutor = Executors.newCachedThreadPool();

        initUI();
        startPolling();
    }

    private void initUI() {
        setPadding(new Insets(15));
        setStyle("-fx-background-color: #f4f4f4;");

        // top: header and global actions
        VBox topBox = new VBox(15);
        topBox.setPadding(new Insets(0, 0, 15, 0));
        
        Label title = new Label("Administration Dashboard");
        title.setStyle("-fx-font-size: 24px; -fx-font-family: 'Segoe UI', Helvetica, Arial; -fx-text-fill: #2c3e50; -fx-font-weight: bold;");
        
        HBox actionBar = new HBox(12);
        actionBar.setAlignment(Pos.CENTER_LEFT);
        
        bucketField = new TextField(config.defaultBucket());
        bucketField.setPrefWidth(120);
        bucketField.setPromptText("bucket");

        Label bucketLabel = new Label("Bucket:");
        bucketLabel.setStyle("-fx-text-fill: #34495e; -fx-font-weight: bold;");

        Button putBtn = createStyledButton("Upload File", "#3498db");
        putBtn.setTooltip(new Tooltip("upload a single local file to the cluster"));
        putBtn.setOnAction(e -> handlePut());

        Button bulkPutBtn = createStyledButton("Upload Many", "#1abc9c");
        bulkPutBtn.setTooltip(new Tooltip("upload multiple files concurrently"));
        bulkPutBtn.setOnAction(e -> handleBulkPut());
        
        Button getBtn = createStyledButton("Download Object (Get)", "#2ecc71");
        getBtn.setTooltip(new Tooltip("download the selected object to your computer"));
        getBtn.setOnAction(e -> handleGet());
        
        Button refreshBtn = createStyledButton("Manual Refresh", "#95a5a6");
        refreshBtn.setTooltip(new Tooltip("manually refresh cluster state and inventory"));
        refreshBtn.setOnAction(e -> refreshData());

        Button reloadBucketBtn = createStyledButton("Load Bucket", "#8e44ad");
        reloadBucketBtn.setTooltip(new Tooltip("refresh the selected bucket inventory"));
        reloadBucketBtn.setOnAction(e -> refreshData());

        actionBar.getChildren().addAll(bucketLabel, bucketField, reloadBucketBtn, putBtn, bulkPutBtn, getBtn, refreshBtn);

        HBox nodeBar = new HBox(12);
        nodeBar.setAlignment(Pos.CENTER_LEFT);

        Button stopManagerBtn = createStyledButton("Stop Manager", "#c0392b");
        stopManagerBtn.setOnAction(e -> handleComposeAction("stop", "manager", "Manager"));

        Button startManagerBtn = createStyledButton("Start Manager", "#27ae60");
        startManagerBtn.setOnAction(e -> handleComposeAction("start", "manager", "Manager"));

        Button stopObjectBtn = createStyledButton("Stop Object", "#c0392b");
        stopObjectBtn.setOnAction(e -> handleComposeAction("stop", "object", "Object Node"));

        Button startObjectBtn = createStyledButton("Start Object", "#27ae60");
        startObjectBtn.setOnAction(e -> handleComposeAction("start", "object", "Object Node"));

        Button stopVolume0Btn = createStyledButton("Stop Volume-0", "#d35400");
        stopVolume0Btn.setOnAction(e -> handleComposeAction("stop", "volume-0", "Volume-0"));

        Button startVolume0Btn = createStyledButton("Start Volume-0", "#16a085");
        startVolume0Btn.setOnAction(e -> handleComposeAction("start", "volume-0", "Volume-0"));

        Button stopVolume1Btn = createStyledButton("Stop Volume-1", "#d35400");
        stopVolume1Btn.setOnAction(e -> handleComposeAction("stop", "volume-1", "Volume-1"));

        Button startVolume1Btn = createStyledButton("Start Volume-1", "#16a085");
        startVolume1Btn.setOnAction(e -> handleComposeAction("start", "volume-1", "Volume-1"));

        nodeBar.getChildren().addAll(stopManagerBtn, startManagerBtn, stopObjectBtn, startObjectBtn, stopVolume0Btn, startVolume0Btn, stopVolume1Btn, startVolume1Btn);

        topBox.getChildren().addAll(title, actionBar);
        topBox.getChildren().add(nodeBar);
        setTop(topBox);

        // center: split view for volumes and objects
        SplitPane splitPane = new SplitPane();
        splitPane.setStyle("-fx-box-border: transparent; -fx-padding: 5;");
        splitPane.setDividerPositions(0.4);

        // left side: volumes topology
        VBox volumeBox = new VBox(10);
        volumeBox.setPadding(new Insets(0, 10, 0, 0));
        Label volLabel = new Label("Cluster Health & Topology");
        volLabel.setStyle("-fx-font-weight: bold; -fx-text-fill: #34495e;");
        
        initVolumeTable();
        volumeBox.getChildren().addAll(volLabel, volumeTable);
        VBox.setVgrow(volumeTable, Priority.ALWAYS);

        // right side: object inventory
        VBox objectBox = new VBox(10);
        objectBox.setPadding(new Insets(0, 0, 0, 10));
        
        HBox inventoryHeader = new HBox(15);
        inventoryHeader.setAlignment(Pos.CENTER_LEFT);
        Label invLabel = new Label("Object Inventory");
        invLabel.setStyle("-fx-font-weight: bold; -fx-text-fill: #34495e;");
        
        searchField = new TextField();
        searchField.setPromptText("search objects...");
        searchField.setPrefWidth(250);
        searchField.setStyle("-fx-background-radius: 15; -fx-border-radius: 15;");
        searchField.textProperty().addListener((obs, old, newValue) -> filterObjects(newValue));
        
        inventoryHeader.getChildren().addAll(invLabel, searchField);
        
        initObjectTable();
        objectBox.getChildren().addAll(inventoryHeader, objectTable);
        VBox.setVgrow(objectTable, Priority.ALWAYS);

        splitPane.getItems().addAll(volumeBox, objectBox);
        setCenter(splitPane);

        // bottom: status bar
        VBox statusArea = new VBox(8);

        HBox statusBox = new HBox();
        statusBox.setPadding(new Insets(10, 0, 0, 0));
        Label statusLabel = new Label("Manager: " + config.managerUrl() + " | Object Node: " + config.objectNodeUrl());
        statusLabel.setStyle("-fx-text-fill: #7f8c8d; -fx-font-size: 11px;");
        statusBox.getChildren().add(statusLabel);

        activityLog = new TextArea();
        activityLog.setEditable(false);
        activityLog.setWrapText(true);
        activityLog.setPrefRowCount(4);
        activityLog.setPromptText("Action log");

        statusArea.getChildren().addAll(statusBox, activityLog);
        setBottom(statusArea);
    }

    private Button createStyledButton(String text, String color) {
        Button btn = new Button(text);
        btn.setStyle("-fx-background-color: " + color + "; -fx-text-fill: white; -fx-font-weight: bold; -fx-padding: 8 15; -fx-background-radius: 5;");
        // simple hover effect
        btn.setOnMouseEntered(e -> btn.setStyle("-fx-background-color: derive(" + color + ", -10%); -fx-text-fill: white; -fx-font-weight: bold; -fx-padding: 8 15; -fx-background-radius: 5; -fx-cursor: hand;"));
        btn.setOnMouseExited(e -> btn.setStyle("-fx-background-color: " + color + "; -fx-text-fill: white; -fx-font-weight: bold; -fx-padding: 8 15; -fx-background-radius: 5;"));
        return btn;
    }

    private void initVolumeTable() {
        volumeTable = new TableView<>(volumeList);
        volumeTable.setStyle("-fx-selection-bar: #ecf0f1; -fx-selection-bar-non-focused: #f5f6fa;");
        
        TableColumn<VolumeNodeModel, String> urlCol = new TableColumn<>("Node URL");
        urlCol.setCellValueFactory(cellData -> new javafx.beans.property.SimpleStringProperty(cellData.getValue().getUrl()));
        
        // status column with visual dot
        TableColumn<VolumeNodeModel, String> statusCol = new TableColumn<>("Status");
        statusCol.setCellValueFactory(cellData -> new javafx.beans.property.SimpleStringProperty(cellData.getValue().getStatus()));
        statusCol.setCellFactory(column -> new TableCell<>() {
            @Override
            protected void updateItem(String item, boolean empty) {
                super.updateItem(item, empty);
                if (empty || item == null) {
                    setGraphic(null);
                    setText(null);
                } else {
                    HBox box = new HBox(5);
                    box.setAlignment(Pos.CENTER_LEFT);
                    Circle dot = new Circle(5, item.equals("ONLINE") ? Color.GREEN : Color.RED);
                    box.getChildren().addAll(dot, new Label(item));
                    setGraphic(box);
                }
            }
        });
        
        // usage column with progress bar
        TableColumn<VolumeNodeModel, Double> usageCol = new TableColumn<>("Storage Usage");
        usageCol.setCellValueFactory(cellData -> new javafx.beans.property.SimpleDoubleProperty(cellData.getValue().getUsagePercent()).asObject());
        usageCol.setCellFactory(column -> new TableCell<>() {
            @Override
            protected void updateItem(Double item, boolean empty) {
                super.updateItem(item, empty);
                if (empty || item == null) {
                    setGraphic(null);
                } else {
                    VBox box = new VBox(2);
                    box.setAlignment(Pos.CENTER);
                    ProgressBar pb = new ProgressBar(item / 100.0);
                    pb.setMaxWidth(Double.MAX_VALUE);
                    if (item > 90) pb.setStyle("-fx-accent: red;");
                    else if (item > 70) pb.setStyle("-fx-accent: orange;");
                    
                    Label label = new Label(String.format("%.1f%%", item));
                    box.getChildren().addAll(pb, label);
                    setGraphic(box);
                }
            }
        });

        volumeTable.getColumns().setAll(List.of(urlCol, statusCol, usageCol));
        volumeTable.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY_FLEX_LAST_COLUMN);
    }

    private void initObjectTable() {
        objectTable = new TableView<>(objectList);
        
        TableColumn<DashboardBackend.ObjectEntryModel, String> pathCol = new TableColumn<>("Path");
        pathCol.setCellValueFactory(cellData -> new javafx.beans.property.SimpleStringProperty(cellData.getValue().path()));
        
        TableColumn<DashboardBackend.ObjectEntryModel, Long> sizeCol = new TableColumn<>("Size (Bytes)");
        sizeCol.setCellValueFactory(cellData -> new javafx.beans.property.SimpleLongProperty(cellData.getValue().size()).asObject());
        
        TableColumn<DashboardBackend.ObjectEntryModel, Long> tsCol = new TableColumn<>("Timestamp");
        tsCol.setCellValueFactory(cellData -> new javafx.beans.property.SimpleLongProperty(cellData.getValue().timestamp()).asObject());

        objectTable.getColumns().setAll(List.of(pathCol, sizeCol, tsCol));
        objectTable.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY_FLEX_LAST_COLUMN);
    }

    private void startPolling() {
        scheduler.scheduleAtFixedRate(this::refreshData, 0, config.pollingIntervalMs(), TimeUnit.MILLISECONDS);
    }

    private void refreshData() {
        try {
            StateReply state = backend.getClusterState();
            List<DashboardBackend.ObjectEntryModel> objects = backend.getObjectInventory(getSelectedBucket());
            Platform.runLater(() -> {
                updateVolumeTable(state);
                updateObjectTable(objects);
                appendLog("Refreshed cluster state and bucket inventory for '" + getSelectedBucket() + "'.");
            });
        } catch (Exception e) {
            appendLog("Refresh failed: " + e.getMessage());
        }
    }

    private void updateVolumeTable(StateReply state) {
        volumeList.clear();
        long now = Instant.now().toEpochMilli();
        for (var node : state.nodes()) {
            String status = (now - node.lastSeenEpochMs() < 20000) ? "ONLINE" : "OFFLINE";
            for (var vinfo : node.vinfo()) {
                double usedPercent = (1.0 - (double)vinfo.availableSize() / (32L * 1024 * 1024)) * 100;
                volumeList.add(new VolumeNodeModel(node.url(), status, usedPercent));
            }
        }
    }

    private void updateObjectTable(List<DashboardBackend.ObjectEntryModel> objects) {
        objectList.setAll(objects);
        filterObjects(searchField.getText());
    }

    private String getSelectedBucket() {
        String bucket = bucketField.getText();
        if (bucket == null || bucket.trim().isEmpty()) {
            return config.defaultBucket();
        }
        return bucket.trim();
    }

    private void filterObjects(String query) {
        if (query == null || query.isEmpty()) {
            objectTable.setItems(objectList);
            return;
        }
        ObservableList<DashboardBackend.ObjectEntryModel> filtered = objectList.filtered(
            obj -> obj.path().toLowerCase().contains(query.toLowerCase())
        );
        objectTable.setItems(filtered);
    }

    private void handlePut() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Select File to Upload");
        File file = fileChooser.showOpenDialog(getScene().getWindow());
        
        if (file != null) {
            if (file.length() > MAX_FILE_SIZE_BYTES) {
                showAlert(Alert.AlertType.WARNING, "File Too Large", "The file exceeds the 4MiB limit.");
                return;
            }

            TextInputDialog dialog = new TextInputDialog(file.getName());
            dialog.initOwner(getScene().getWindow());
            dialog.setTitle("Remote Object Name");
            dialog.setHeaderText("Specify the path for this object:");
            dialog.setContentText("Path:");

            var result = dialog.showAndWait();
            if (result.isPresent()) {
                uploadSingleFile(file, result.get(), false);
            }
        }
    }

    private void handleBulkPut() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Select Files to Upload");
        List<File> files = fileChooser.showOpenMultipleDialog(getScene().getWindow());

        if (files == null || files.isEmpty()) {
            return;
        }

        TextInputDialog dialog = new TextInputDialog("");
        dialog.initOwner(getScene().getWindow());
        dialog.setTitle("Remote Path Prefix");
        dialog.setHeaderText("Optional prefix for uploaded objects:");
        dialog.setContentText("Prefix:");

        String prefix = dialog.showAndWait().orElse("").trim();
        String bucket = getSelectedBucket();

        appendLog("Starting bulk upload of " + files.size() + " file(s) to bucket '" + bucket + "'.");

        AtomicInteger successCount = new AtomicInteger();
        AtomicInteger failureCount = new AtomicInteger();
        List<CompletableFuture<Void>> futures = new ArrayList<>();

        for (File file : files) {
            futures.add(CompletableFuture.runAsync(() -> {
                boolean success = uploadSingleFile(file, composeRemotePath(prefix, file.getName()), true, bucket);
                if (success) {
                    successCount.incrementAndGet();
                } else {
                    failureCount.incrementAndGet();
                }
            }, actionExecutor));
        }

        CompletableFuture.allOf(futures.toArray(new CompletableFuture[0]))
            .whenComplete((ignored, throwable) -> Platform.runLater(() -> {
                if (throwable == null) {
                    appendLog("Bulk upload finished: " + successCount.get() + " succeeded, " + failureCount.get() + " failed.");
                    refreshData();
                } else {
                    appendLog("Bulk upload failed: " + throwable.getMessage());
                    showAlert(Alert.AlertType.ERROR, "Bulk Upload Error", "One or more uploads failed.");
                }
            }));
    }

    private String composeRemotePath(String prefix, String fileName) {
        if (prefix == null || prefix.isBlank()) {
            return fileName;
        }
        if (prefix.endsWith("/")) {
            return prefix + fileName;
        }
        return prefix + "/" + fileName;
    }

    private boolean uploadSingleFile(File file, String remotePath, boolean background, String... bucketOverride) {
        String bucket = bucketOverride.length > 0 ? bucketOverride[0] : getSelectedBucket();
        if (file.length() > MAX_FILE_SIZE_BYTES) {
            appendLog("Skipped '" + file.getName() + "' because it exceeds the 4MiB limit.");
            return false;
        }

        try {
            byte[] data = Files.readAllBytes(file.toPath());
            int status = backend.putObject(bucket, remotePath, data);
            if (status == 0) {
                appendLog("Uploaded '" + remotePath + "' from '" + file.getName() + "'.");
                if (!background) {
                    Platform.runLater(this::refreshData);
                }
                return true;
            }

            appendLog("Upload failed for '" + remotePath + "'.");
            return false;
        } catch (Exception e) {
            appendLog("Could not upload '" + file.getName() + "': " + e.getMessage());
            return false;
        }
    }

    private void handleGet() {
        DashboardBackend.ObjectEntryModel selected = objectTable.getSelectionModel().getSelectedItem();
        if (selected == null) {
            showAlert(Alert.AlertType.WARNING, "Warning", "Select an object first.");
            return;
        }

        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Save Object As");
        fileChooser.setInitialFileName(selected.path());
        File file = fileChooser.showSaveDialog(getScene().getWindow());

        if (file != null) {
            byte[] data = backend.getObject(selected.bucket(), selected.path());
            if (data != null) {
                try {
                    Files.write(file.toPath(), data);
                    showAlert(Alert.AlertType.INFORMATION, "Success", "Downloaded.");
                } catch (Exception e) {
                    showAlert(Alert.AlertType.ERROR, "Error", "failed to save.");
                }
            }
        }
    }

    private void handleComposeAction(String action, String service, String label) {
        actionExecutor.submit(() -> {
            appendLog(action + " requested for " + label + ".");
            int exitCode = runComposeCommand(action, service);
            if (exitCode == 0) {
                appendLog(label + " " + action + " completed.");
            } else {
                appendLog(label + " " + action + " failed with exit code " + exitCode + ".");
            }
            refreshData();
        });
    }

    private int runComposeCommand(String action, String service) {
        List<String> command = List.of(
            "docker",
            "compose",
            "-f",
            COMPOSE_FILE.toString(),
            action,
            service
        );

        ProcessBuilder processBuilder = new ProcessBuilder(command);
        processBuilder.redirectErrorStream(true);

        try {
            Process process = processBuilder.start();
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    appendLog("[docker] " + line);
                }
            }
            return process.waitFor();
        } catch (Exception e) {
            appendLog("Failed to run docker compose command: " + e.getMessage());
            return 1;
        }
    }

    private void appendLog(String message) {
        Platform.runLater(() -> {
            if (activityLog != null) {
                if (!activityLog.getText().isEmpty()) {
                    activityLog.appendText("\n");
                }
                activityLog.appendText("[" + Instant.now() + "] " + message);
            }
        });
    }

    private void showAlert(Alert.AlertType type, String title, String content) {
        Alert alert = new Alert(type);
        alert.initOwner(getScene().getWindow());
        alert.setTitle(title);
        alert.setHeaderText(null);
        alert.setContentText(content);
        alert.showAndWait();
    }

    public void stopPolling() {
        scheduler.shutdownNow();
        actionExecutor.shutdownNow();
    }

    public static class VolumeNodeModel {
        private final String url;
        private final String status;
        private final double usagePercent;

        public VolumeNodeModel(String url, String status, double usagePercent) {
            this.url = url;
            this.status = status;
            this.usagePercent = usagePercent;
        }

        public String getUrl() { return url; }
        public String getStatus() { return status; }
        public double getUsagePercent() { return usagePercent; }
    }
}
