# IC - Project 2

## Part I - BitStream Class

The `BitStream` class is responsible for efficient bit manipulations, allowing reading and writing of individual bits and sequences of bits in binary files. This class is used for low-level operations necessary for data encoding and decoding.

### Compile and Run

In the directory, execute the following commands to get results:

```bash
cd part1/
make run
```

If you want to delete the outputs of the execution, you can write the following commands:

```bash
make clean
```

## Part II - Golomb Coding

In this part, we developed a C++ class for Golomb coding, a type of universal code often used in data compression. Golomb coding is particularly effective for encoding integers with a geometric distribution and can handle both positive and negative values.

### Compile and Run

In the directory, execute the following commands:

```bash
cd part2/
make run
```

If you want to delete the outputs of the execution, you can write the following commands:

```bash
make clean
```

## Part IV

## Compile and Run
In the directory, execute the following commands:

```bash
cd part4/
make
```

It's necessary to have an image in the directory to be loaded in the program.

### Encode
To encode, in this case, the original.png is the loaded image and the output will be the binary file after encoding.

```bash
./main encode original.png output
```

### Decode
To decode it's necessary to load the binary file in the directory and then run the following commands. It is also necessary to define the final name of the output when decoded.

```bash
./main decode output final.png
```
<i>In this version, the output of decode has to be .png but we will try to change it.</i>


# Authors

[João Monteiro](https://github.com/joaomonteir0), 102690 \
[João Sousa](https://github.com/jsousa11), 103415 \
[João Gaspar](https://github.com/joaogasparp), 107708 
