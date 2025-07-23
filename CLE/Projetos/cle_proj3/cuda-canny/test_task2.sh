#!/bin/bash
# ./test_task2.sh [imagem]

IMAGE=${1:-"images/lake.pgm"}

make clean > /dev/null 2>&1
make

echo "teste nms: $IMAGE"
echo

./canny -i "$IMAGE"

for sigma in 0.5 1.0 1.5; do
    echo
    ./canny -i "$IMAGE" -s $sigma
done