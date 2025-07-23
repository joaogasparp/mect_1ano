#!/bin/bash
# ./test_task3.sh [imagem]

IMAGE=${1:-"images/lake.pgm"}

make clean > /dev/null 2>&1
make

echo "teste edges: $IMAGE"
echo

./canny -i "$IMAGE"

echo
./canny -i "$IMAGE" -x 40

echo
./canny -i "$IMAGE" -x 60