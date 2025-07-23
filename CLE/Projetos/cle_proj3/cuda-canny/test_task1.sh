#!/bin/bash
# ./test_task1.sh [imagem]

IMAGE=${1:-"images/lake.pgm"}

echo "teste gaussian filter: $IMAGE"
echo

./canny -i "$IMAGE"

echo
./canny -i "$IMAGE" -s 0.5

echo
./canny -i "$IMAGE" -s 1.5

echo  
./canny -i "$IMAGE" -s 2.0
