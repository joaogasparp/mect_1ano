#!/bin/bash
# ./test_final.sh [imagem] [sigma] [tmin] [tmax]

IMAGE=${1:-"images/lake.pgm"}
SIGMA=${2:-1.0}
TMIN=${3:-45}
TMAX=${4:-50}

if [ ! -f canny ]; then
    make
fi

./canny -i "$IMAGE" -s $SIGMA -n $TMIN -x $TMAX