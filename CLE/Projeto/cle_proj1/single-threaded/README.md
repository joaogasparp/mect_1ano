# Practical Assignment 1 - Single Threaded

This project contains the single-threaded implementations of the following exercises:
- **Weather Stations**
- **Word Count**

This version serves as a foundation for future multi-threaded implementations.

## Setup Instructions

### Requirements
- **Platform:** Linux/Unix
- **Compiler:** C++ with support for C++11 or higher
- **Tools:** CMake and Make

### Project Structure
The `single_threaded` directory includes:
- **weather_stations/**
  - `src/` - Source code (e.g., `main.cpp`, `cities.cpp`, `create-samples.cpp`)
  - `CMakeLists.txt` and `Makefile` - Build configurations
  - `build/` - Output directory for executables (e.g., `cle-ws`, `cle-samples`)
  - `samples-1000000000.txt` - Text file with 1 billion samples
- **word_count/**
  - `src/` - Source code (e.g., `main.cpp`, `word_count.cpp`, `utf-8.cpp`)
  - `CMakeLists.txt` and `Makefile` - Build configurations
  - `build/` - Output directory for the executable (e.g., `cle-wc`)
  - `books/` - Directory with all the books
  - `files.txt` - The input file used to test the code, it has all the books listed

### Compilation
To build either project, navigate to the desired project's directory and execute:

```bash
make
```

The executables will be generated in the `build/` directory.

## Usage Details

### Weather Stations

#### Sample Data Generation
A program (`cle-samples`) is provided to generate sample data. Usage:

```bash
./build/cle-samples NUMBER_OF_SAMPLES
```

This command creates a file named `samples-NUMBER_OF_SAMPLES.txt` containing random temperature measurements based on real city data.

#### Execution
To run the Weather Stations program:

```bash
./build/cle-ws <input_file>
```

### Word Count

#### Execution
To run the Word Count program:

```bash
./build/cle-wc <input_file>
```

## Summary of Approach

- **Weather Stations:**  
  The program reads an input file of weather measurements, calculates the average, minimum, and maximum temperatures for each station, and outputs the results sorted alphabetically. The implementation utilizes the C++ Standard Library and CMake for build configuration.

- **Word Count:**  
  The program processes a list of text files, counting the number of characters, lines, and words in each file individually. It includes a UTF-8 processor to ensure correct interpretation of the text.
