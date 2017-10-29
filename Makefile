build: image_processing.c
	gcc -Wall main2.c -o image_processing

run: image_processing
	./image_processing

clean:
	rm -rf *.o image_processing
