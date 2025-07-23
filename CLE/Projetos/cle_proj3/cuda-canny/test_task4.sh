#!/bin/bash
# ./test_task4.sh [imagem]

IMAGE=${1:-"images/lake.pgm"}

make clean > /dev/null 2>&1
make

echo "teste completo: $IMAGE"
echo

./canny -i "$IMAGE"

echo
./canny -i "$IMAGE" -n 30 -x 40

echo
./canny -i "$IMAGE" -n 50 -x 70