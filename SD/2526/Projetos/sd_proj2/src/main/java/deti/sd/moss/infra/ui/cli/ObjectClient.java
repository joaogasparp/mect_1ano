package deti.sd.moss.infra.ui.cli;

import java.io.IOException;
import java.io.File;
import java.nio.file.Files;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.Callable;

import picocli.CommandLine;
import picocli.CommandLine.Option;
import picocli.CommandLine.Command;

import deti.sd.moss.infra.rpc.discovery.GrpcServiceDiscovery;
import deti.sd.moss.core.object.ports.ObjectService;
import deti.sd.moss.core.object.model.GetRequest;
import deti.sd.moss.core.object.model.PutRequest;
import deti.sd.moss.core.object.model.ListRequest;

@Command(name = "put")
class PutCommand implements Callable<Integer> {

    @CommandLine.ParentCommand
    private ObjectClient parent;

    @Option(names = "-b", defaultValue = "sd")
    String bucket;
    @CommandLine.Parameters(index = "0")
    File file;
    @CommandLine.Parameters(index = "1")
    String path;

    @Override
    public Integer call() throws IOException {

        // Validate user input
        if (!file.exists()) {
            System.err.printf("Error: File '%s' not found.%n", file.toPath());
            return 1; // Return non-zero exit code for errors
        }

        if (file.isDirectory()) {
            System.err.printf("Error: '%s' is a directory, not a file.%n", file.toPath());
            return 1; // Return non-zero exit code for errors
        }

        if (file.length() > 4 * 1024 * 1024) {
            System.err.printf("Error: File '%s' is too large.%n", file.toPath());
            return 1; // Return non-zero exit code for errors
        }
        // --------------------

        var discovery = new GrpcServiceDiscovery();
        var objects = discovery.getObject(parent.remote);

        var response = objects.put(new PutRequest(
                bucket,
                path,
                Files.readAllBytes(file.toPath())));

        if (response.status() != 0) {
            System.err.println("Failed to put object");
            return 1;
        }

        return 0;
    }

}

@Command(name = "get")
class GetCommand implements Callable<Integer> {

    @CommandLine.ParentCommand
    private ObjectClient parent;

    @Option(names = "-b", defaultValue = "sd")
    String bucket;
    @CommandLine.Parameters(index = "0")
    String path;
    @CommandLine.Parameters(index = "1")
    File file;

    @Override
    public Integer call() throws IOException {
        if (file.exists()) {
            System.err.printf("Error: File '%s' already exists.%n", file.toPath());
            return 1;
        }

        if (file.isDirectory()) {
            System.err.printf("Error: '%s' is a directory, not a file.%n", file.toPath());
            return 1;
        }

        var discovery = new GrpcServiceDiscovery();
        var objects = discovery.getObject(parent.remote);

        var response = objects.get(new GetRequest(
                bucket,
                path));

        if (response.status() != 0) {
            System.err.println("Failed to get object");
            return 1;
        }

        Files.write(file.toPath(), response.data());
        return 0;
    }
}

@Command(name = "list")
class ListCommand implements Callable<Integer> {

    @CommandLine.ParentCommand
    private ObjectClient parent;

    @Option(names = "-b", defaultValue = "sd")
    String bucket;

    @Override
    public Integer call() throws IOException {
        var discovery = new GrpcServiceDiscovery();
        var objects = discovery.getObject(parent.remote);

        var response = objects.list(new ListRequest(bucket));

        if (response.status() != 0) {
            System.err.println("Failed to list bucket(s)");
            return 1;
        }

        for (var bucket : response.bucketInfo()) {
            for (var item : bucket) {
                System.out.println(item);
            }
        }

        return 0;
    }

}

@Command(name = "moss", mixinStandardHelpOptions = true, version = "SD.26", description = "MOSS client", subcommands = {
        PutCommand.class, GetCommand.class, ListCommand.class }) // Register subcommands here
public class ObjectClient implements Callable<Integer> {

    // ==========================================================================
    @Option(names = { "-r", "--remote" }, description = "Remote object server", defaultValue = "localhost:4281")
    public String remote;

    @Override
    public Integer call() throws Exception {
        // This runs if no subcommand is provided
        CommandLine.usage(this, System.out);
        return 0;
    }

}
