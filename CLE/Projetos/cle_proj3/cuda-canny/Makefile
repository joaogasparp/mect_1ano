
canny: src/canny.cu src/image.c src/canny-device.cu
	nvcc -O2 src/canny.cu -o $@

.PHONY: clean
clean:
	@echo "[RM] canny"
	@rm -f canny
	@echo "[RM] *.pgm"
	@rm -f *.pgm
