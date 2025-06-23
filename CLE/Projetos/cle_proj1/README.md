# Practical Assignment 1

This project contains implementations for word count and weather station data processing in both single-threaded and multi-threaded versions.

## Setup

### Compilation

We only show how to do it with word-count. You have to do the same with weather-stations, change the name of the folder when you do the `cd` command.

#### Multi-threaded

1. Navigate to the project directory:
    ```sh
    cd multi-threaded/word-count
    ```

2. Create the build directory and navigate to it:
    ```sh
    mkdir build && cd build
    ```

3. Run CMake and Make:
    ```sh
    cmake ..
    make
    cd ..
    ```

#### Single-threaded

1. Navigate to the project directory:
    ```sh
    cd single-threaded/word-count
    ```

2. Create the build directory and navigate to it:
    ```sh
    mkdir build && cd build
    ```

3. Run CMake and Make:
    ```sh
    cmake ..
    make
    cd ..
    ```

## Usage

### Word Count

#### Multi-threaded

1. Run the program with the file list:
    ```sh
    ./build/cle-wc <input_file> <num_threads>
    ```

#### Single-threaded

1. Run the program with the file list:
    ```sh
    ./build/cle-wc <input_file>
    ```

### Weather Stations

#### Generating Sample Data

1. To generate sample data use (we recommend 10.000.000 without the dots):
    ```sh
    ./build/cle-samples NUMBER_OF_SAMPLES
    ```

The generated file will be used as input for the Weather Stations processing.

#### Multi-threaded

1. Run the program with the data file and number of threads:
    ```sh
    ./build/cle-ws <input_file> <num_threads>
    ```

#### Single-threaded

1. Run the program with the data file:
    ```sh
    ./build/cle-ws <input_file>
    ```

## Approach

### Word Count

The word count implementation reads a list of files and counts the number of lines, words, and characters in each file. The multi-threaded version uses multiple threads to process files in parallel, while the single-threaded version processes files sequentially.

### Weather Stations

The weather station implementation processes temperature data from various stations. The multi-threaded version divides the data file into blocks and processes each block in a separate thread, while the single-threaded version processes the file sequentially.

Both implementations use efficient data structures and synchronization techniques to ensure accuracy and processing efficiency.
