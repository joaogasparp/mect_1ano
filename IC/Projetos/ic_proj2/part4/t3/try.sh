echo "Encoding..."
./main encode teste.y4m output.bin
echo "Deconding..."
./main decode output.bin teste.mp4
echo "Comparing original and reconstructed..."
./videoTester teste.y4m  teste.mp4