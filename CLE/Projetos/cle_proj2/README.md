# Practical Assignment 2

## Summary of Approach

In this assignment, we implemented two main applications: **Weather Stations** and **Word Count**, both in single-threaded and MPI-based parallel versions. The goal was to process large datasets efficiently on a multi-core or multi-node Linux/Unix cluster.

### Weather Stations

The **Weather Stations** application processes weather data to compute statistics (average, minimum, and maximum temperatures) for multiple weather stations. The MPI implementation divides the dataset among processes, where each process computes local statistics. The results are then gathered and consolidated by the master process (rank 0), which also sorts the stations by maximum temperature.

### Word Count

The **Word Count** application counts the number of characters, lines, and words in a set of files. The MPI implementation distributes the files among processes, where each process computes local counts. The results are gathered by the master process, which consolidates and displays the final counts for all files.

### Performance Analysis

We conducted a performance analysis to compare the single-threaded and MPI implementations. The analysis includes metrics such as execution time, speedup, and efficiency for different input sizes and numbers of processes. A [brief report](cle_proj2.pdf) summarizing the findings is included in the project.  

---

## Setup and Usage

### Single Threaded

1. Navigate to the single-threaded directory:
    ```sh
    cd single-threaded/
    ```

#### Weather Stations

1. Navigate to the project directory:
    ```sh
    cd weather-stations/
    ```

2. Run Make:
    ```sh
    make
    ```

3. Run the program with the file list:
    ```sh
    ./build/cle-ws <input_file>

##### Generating Sample Data

1. To generate sample data use (we recommend 10.000.000 without the dots):
    ```sh
    ./build/cle-samples NUMBER_OF_SAMPLES
    ```

The generated file will be used as input for the Weather Stations processing.

#### Word Count

1. Navigate to the project directory:
    ```sh
    cd word-count/
    ```

2. Run Make:
    ```sh
    make
    ```

3. Run the program with the file list:
    ```sh
    ./build/cle-wc <input_file>

### MPI Implementations

1. Navigate to the mpi-implementations directory:
    ```sh
    cd mpi-implementations/
    ```

#### Weather Stations

1. Navigate to the project directory:
    ```sh
    cd weather-stations/
    ```

2. Run Make:
    ```sh
    make
    ```

3. Run the program with the file list:
    ```sh
    mpirun -np <num_processes> ./build/cle-ws-mpi <input_file>

##### Generating Sample Data

1. To generate sample data use (we recommend 10.000.000 without the dots):
    ```sh
    ./build/cle-samples NUMBER_OF_SAMPLES
    ```

The generated file will be used as input for the Weather Stations processing.

#### Word Count

1. Navigate to the project directory:
    ```sh
    cd word-count/
    ```

2. Run Make:
    ```sh
    make
    ```

3. Run the program with the file list:
    ```sh
    mpirun -np <num_processes> ./build/cle-wc-mpi <input_file>
    ```