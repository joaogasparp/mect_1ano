for i in {0..5}; do
    ./main encode lena.ppm output$i.bin $i
    ./main decode output$i.bin lena$i.png $i
done