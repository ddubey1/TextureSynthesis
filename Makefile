CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -g

# Creates executables for running and testing.
project: project.o ppm.o image.o texture_synthesis.o
	$(CC) -o project project.o ppm.o image.o texture_synthesis.o -lm

# Creates object files from .c files.
project.o: project.c ppm.h image.h texture_synthesis.h
	$(CC) $(CFLAGS) -c project.c -lz -lm

texture_synthesis.o: texture_synthesis.c texture_synthesis.h image.h
	$(CC) $(CFLAGS) -c texture_synthesis.c -lz -lm

ppm.o: ppm.c ppm.h image.h 
	$(CC) $(CFLAGS) -c ppm.c 

image.o: image.c image.h
	$(CC) $(CFLAGS) -c image.c


# Gets rid of object files and executables.
clean:
	rm -f *.o main
