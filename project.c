#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "image.h"
#include "ppm.h"
#include "texture_synthesis.h"

// how to run executable for testing ./project data/D1.ppm tests/D1_test_2.ppm 128 128 2

int main( int argc , char *argv[] )
{
	// Seed the random number generator so that the code produces the same results on the same input.
	srand(0);

	// Check if the number of arguments is correct
	int num_arguments = argc;
	if (argc != 6) {
		printf("Error: incorrect number of command line arguments. Please give 6 arguments instead of %d.\n", num_arguments);
		return 1;
	}

	// Assign command line arguments to variables
	unsigned int outWidth = atoi(argv[3]);
	unsigned int outHeight = atoi(argv[4]);
	unsigned int radius = atoi(argv[5]);

	// Get the time at the start of execution
	clock_t start_clock = clock();

	// check if input/output filenames are correct
	

	// open input/output files with the corresponding filenames
	FILE *in = fopen(argv[1], "r");
	if (in == NULL) {
		printf("Error: could not open input file.\n");
		return 2;
	}

	FILE *out = fopen(argv[2], "w");
	// check if output file cannot be opened
	if (out == NULL) {
		printf("Error: could not open output file.\n");
		return 4;
	}

	Image * exemplar = ReadPPM(in);
	
	if (exemplar == NULL) {
		printf("Error: could not read input file.\n");
		return 3;
	}


	// Check if dimensions of exemplar are less than output image
	if(exemplar->width > outWidth && exemplar->height > outHeight) {
		return 2;
	}

	Image * synthesized = SynthesizeFromExemplar(exemplar, outWidth, outHeight, radius, 0);

	// Write ppm image to file. If there is an error 
	int error = WritePPM(out, synthesized);
	if (error == -1) {
		printf("Error: could not write to file.");
		return 4;
	}

	// Close input/output files
	fclose(in);
	fclose(out);

	// free image
	FreeImage(&exemplar);
	FreeImage(&synthesized);

	// Get the time at the end of the execution
	clock_t clock_difference = clock() - start_clock;

	// Convert the time difference into seconds and print
	printf( "Synthesized texture in %.2f(s)\n" , (double)clock_difference/CLOCKS_PER_SEC );
	return 0;
}

