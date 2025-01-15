## Details:
O código usa o predictor JPEG-LS
```bash
predictJPEG_LS(int a, int b, int c) {
    int pred = a + b - c;
    int minVal = std::min(a, b);
    int maxVal = std::max(a, b);

    if (c >= maxVal) {
        pred = minVal;
    } else if (c <= minVal) {
        pred = maxVal;
    }
    return pred;
}

```

## Usage:

```bash
make clean && make

./main <encode|decode> <input> <output>
```

### Encode:
```bash
./main encode teste.y4m output.bin
```
### Decode:
```bash
./main decode output.bin output.mp4 
```

### Compare:
```bash
./videoTester teste.y4m  output.mp4
```

### Testing:
```bash
bash try.sh
```