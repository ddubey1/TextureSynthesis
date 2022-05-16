#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "image.h"
#include "texture_synthesis.h"

// compares tbs pixels 
int CompareTBSPixels( const void *v1 , const void *v2 )
{
	const TBSPixel *tp1 = (const TBSPixel *)v1;
	const TBSPixel *tp2 = (const TBSPixel *)v2;
	int d = tp1->neighborCount - tp2->neighborCount;
	if( d ) return -d;
	d = tp1->idx.y - tp2->idx.y;
	if( d ) return d;
	d = tp1->idx.x - tp2->idx.x;
	if( d ) return d;
	return tp2->r - tp1->r;
}

// sorts tbs pixels, returns zero if succeeded
int SortTBSPixels( TBSPixel *tbsPixels , unsigned int sz )
{
	unsigned int *permutation = (unsigned int*)malloc( sizeof(unsigned int)*sz );
	if( !permutation )
	{
		fprintf( stderr , "[ERROR] Failed to allocate memory for permutation: %d\n" , sz );
		return 1;
		exit(1);
	}
	for( unsigned int i=0 ; i<sz ; i++ ) permutation[i] = i;
	for( unsigned int i=0 ; i<sz ; i++ )
	{
		unsigned int i1 = rand() % sz;
		unsigned int i2 = rand() % sz;
		unsigned int tmp = permutation[i1];
		permutation[i1] = permutation[i2];
		permutation[i2] = tmp;
	}

	for( unsigned int i=0 ; i<sz ; i++ ) tbsPixels[i].r = permutation[i];
	free( permutation );

	qsort( tbsPixels , sz , sizeof( TBSPixel ) , CompareTBSPixels );

	return 0;
}

// Synthesizes output image from exemplar image
Image *SynthesizeFromExemplar( const Image *exemplar , unsigned int outWidth , unsigned int outHeight , unsigned int windowRadius , bool verbose )
{
	// output image pointer initialized to null
	Image *synthesized = NULL;
	
	// set parameters for synthesized
	synthesized = AllocateImage(outWidth, outHeight);

	// TESTING: setting all pixels to grey first for visibility (only for testing purposes)
	if (verbose == 1) {
		for( unsigned int i=0 ; i<(unsigned int)( (outWidth)*(outHeight) ) ; i++ ) {
			synthesized->pixels[i].r = 50;
			synthesized->pixels[i].g = 50;
			synthesized->pixels[i].b = 50;

			// even though pixel is grey, still considered unset
			synthesized->pixels[i].a = 0;
		}
	}

	// sets all pixels in black and unset if verbose is 0;
	else {
			for( unsigned int i=0 ; i<(unsigned int)( (outWidth)*(outHeight) ) ; i++ ) {
				synthesized->pixels[i].r = 0;
				synthesized->pixels[i].g = 0;
				synthesized->pixels[i].b = 0;
				synthesized->pixels[i].a = 0;
		}
	}
	

	// setting top left corner to the exemplar image
	int ex_h = exemplar->height;
	int ex_w = exemplar->width;

	// for every pixel in the exemplar image
	for(unsigned int i=0 ; i<(unsigned int)(ex_h) ; i++) {
		for (unsigned int j=0 ; j<(unsigned int)(ex_w) ; j++) {
			
			// pixel in the exemplar
			Pixel ex_pixel = exemplar->pixels[ i*ex_w + j ];

			// corresponding in the synthesized image
			Pixel *img_pixel = &(synthesized->pixels[ i*outWidth + j ]);

			// set the color of the synthesized pixel to the color of the exemplar pixel
			// in the width and height of the exemplar
			setPixel(img_pixel, ex_pixel);
		}
	}
	
	// synthesize all pixels
	synthesizeTexture(synthesized, ex_w, ex_h, outWidth, outHeight, windowRadius);

	return synthesized;
}

// Takes a pointer to the old pixel and the new pixel itself
// Sets the old pixel to the new pixel
void setPixel(Pixel * old_color, const Pixel new_color){

	// breaks if the old pixel is already set
	assert(!(old_color->a == 255));

	// sets the old pixel's colors to the new pixel's colors
	old_color->r = new_color.r;
	old_color->g = new_color.g;
	old_color->b = new_color.b;
	old_color->a = 255;
	
}

// Takes in a pointer to the image to be synthesized, the width of the output image, the height
// of the output image, and an int pointer to size (which will later be set to the size of the
// TBSPixel array). Finds all the to-be-set pixels and adds and returns them in an array.
TBSPixel *findTBSPixel(Image *synthesized, unsigned int Width , unsigned int Height, int* size) {
	
	// dynamically allocate array
	TBSPixel* TBSPixelArr = malloc(sizeof(TBSPixel) * Width * Height);

	// keeps track of size of TBSPixelArr
	int counter = 0;

	// for every pixel in the synthesized image
	for(unsigned int i=0 ; i<(unsigned int) (Height) ; i++) {
		
		for (unsigned int j=0 ; j<(unsigned int) (Width) ; j++) {
			
			if(synthesized->pixels[i*Width + j].a != 255){
				
				// Find pixel index and store in in struct
				TBSPixelArr[counter].idx.x = j;
				TBSPixelArr[counter].idx.y = i;
				
				// Check how many pixels up down left right and diagonals
				int neighborCount = findNumNeigbhors(synthesized, (i*Width+j), Width, Height);
				
				// only add the TBSPixel to the array if the neighbour count is not 0
				if (neighborCount != 0) {
					TBSPixelArr[counter].neighborCount = neighborCount;
					
					// Assign random number
					TBSPixelArr[counter].r = rand();
					counter++;
				}

		
			}
		}
	}

	// set the size equal to the counter
	*size = counter;

	return TBSPixelArr;

}

// Checks if index of a pixel in the 1-D form is in bounds
int checkIndexInBound(int index, unsigned int Width, unsigned int Height) {
	if (index >= 0 && index <= (int)(Width * Height - 1)){
		return 1;
	}
	else {
		return 0;
	}
}

// Finds the number of neighbours of a TBSPixel given the
// image to be synthesized, the pixel's index, the output image
// width, and height
int findNumNeigbhors(Image *synthesized, int index, unsigned int Width, unsigned int Height) {
	int counter = 0;
	
	// Up
	if (checkIndexInBound(index - Width, Width, Height)) {
		if(synthesized->pixels[index-Width].a == 255) {
			counter++;
		}
	}

	// Down
	if (checkIndexInBound(index + Width, Width, Height)) {
		if(synthesized->pixels[index+Width].a == 255) {
			counter++;
		}
	}

	// Left
	if (checkIndexInBound(index - 1, Width, Height)) {
		if(synthesized->pixels[index-1].a == 255) {
			counter++;
		}
	}

	// Right
	if (((index+1)%Width != 0) && checkIndexInBound(index + 1, Width, Height)) {
		if(synthesized->pixels[index+1].a == 255) {
			counter++;
		}
	}

	// NE
	if (((index+1)%Width != 0) && checkIndexInBound(index - Width + 1, Width, Height)) {
		if(synthesized->pixels[index-Width + 1].a == 255) {
			counter++;
		}
	}

	// NW
	if (checkIndexInBound(index - Width - 1, Width, Height)) {
		if(synthesized->pixels[index-Width-1].a == 255) {
			counter++;
		}
	}

	// SE
	if (((index+1)%Width != 0) && checkIndexInBound(index + Width + 1, Width, Height)) {
		if(synthesized->pixels[index+Width+1].a == 255) {
			counter++;
		}
	}

	// SW
	if (checkIndexInBound(index + Width - 1, Width, Height)) {
		if(synthesized->pixels[index+Width-1].a == 255) {
			counter++;
		}
	}

	return counter;

}

// Synthesizes the texture of all the TBS Pixels in the output image
// Takes in the image to be synthesized, the exemplar image width, the
// exemplar image height, the output image width, the output image height,
// and the window radius
void synthesizeTexture(Image *synthesized , unsigned int exWidth , unsigned int exHeight , 
						unsigned int synWidth, unsigned int synHeight, unsigned int windowRadius) {
	
	int num_tbs_pixels = 0;

	// Creates inital array of TBS pixels
	TBSPixel * TBSPixelArr = findTBSPixel(synthesized, synWidth, synHeight, &num_tbs_pixels);

	// Iterates while there are still pixels to-be-set
	while (num_tbs_pixels > 0) { 
		// sorts TBS array
		SortTBSPixels(TBSPixelArr, num_tbs_pixels);
		
		// synthesizes the pixel at the first index of TBSPixelArr (has the most neighbors since sorted)
		synthesizePixel(TBSPixelArr, synthesized, exWidth, exHeight, synWidth, synHeight, windowRadius);

		free(TBSPixelArr);

		// creates a new TBS pixel array taking into account the pixel that was just set
		TBSPixelArr = findTBSPixel(synthesized, synWidth, synHeight, &num_tbs_pixels);
	}

	free(TBSPixelArr);					
	
}


// Synthesizes a pixel based on the exemplar and already set pixels
// takes a TBSPixel array, the output image + associated dimensions, the exemplar dimensions and the radius
void synthesizePixel(TBSPixel* TBSPixelArr, Image *synthesized , unsigned int exWidth , unsigned int exHeight , 
						unsigned int synWidth, unsigned int synHeight, unsigned int windowRadius) {
	

	// Finds the x and y coordinates of the pixel with the most amount of neighbors
	unsigned int x_coord = TBSPixelArr->idx.x;
	unsigned int y_coord = TBSPixelArr->idx.y;
	
	// Pointer to pixel with most amount of neighbors
	Pixel* old_pixel = GetPixel(synthesized, TBSPixelArr->idx);
	
	// set a 2D array of pixel pointers around a center pixel which is the first element in the TBSPixelArr (most neighbors)
	Pixel* tbsPixelWindow[2*windowRadius+1][2*windowRadius+1];
	tbsPixelWindow[windowRadius][windowRadius] = old_pixel; // set center of window to the to-be-set pixel
	
	// Sets the rest of the elments of the pixel window to their corresponding pixel around the center pixel
	createPixelWindow(tbsPixelWindow[0], synthesized,
						synWidth, synHeight, synWidth, windowRadius, y_coord, x_coord);
	
	
	//array of Exemplar pixels that are elligible candidates for the TBS pixel
	EXPPixel* EXPPixelArr = malloc(sizeof(EXPPixel) * exHeight * exWidth);
	int counter = 0;

	// Selecting best pixel in the exemplar 
	// Creating a window around every pixel in the exemplar
	for (int i = 0; i < (int)exHeight; i++) {
		for(int j = 0; j < (int)exWidth; j++) {

			// setting a 2D array of pixels pointers, or a window, around each pixel in the exemplar 
			Pixel* expPixelWindow[2*windowRadius+1][2*windowRadius+1];
			expPixelWindow[windowRadius][windowRadius] = &(synthesized->pixels[i * synWidth + j]); // set center to exemplar pixel to be checked

			// setting the pixels in the window to their corresponding pixel values
			createPixelWindow(expPixelWindow[0], synthesized, exWidth, exHeight, synWidth, windowRadius, i, j);

			// checks if the exemplar pixel window is a valid comparison to the TBS pixel window
			// if so, the exemplar pixel is added to the exemplar pixel array and its gaussian is calculated
			int different = 0;
			for(unsigned int h = 0; h < 2*windowRadius+1; h++) {
				for(unsigned int k = 0; k < 2*windowRadius+1; k++) {
					if(expPixelWindow[h][k] == NULL && tbsPixelWindow[h][k] != NULL) {
						different++;
					}
				}
			}
			if(different == 0) {
				EXPPixelArr[counter].idx.x = j;
				EXPPixelArr[counter].idx.y = i;
				EXPPixelArr[counter].GaussScore = findGaussScore(tbsPixelWindow[0], expPixelWindow[0], windowRadius);
				counter++;
			}
		}
	}

	// finding the best exemplar pixel, e.g. the one to set the TBS pixel to
	EXPPixel BestPixel = findBestExemplarPix(EXPPixelArr, counter);
	
	// setting the pixel 
	int x_new = BestPixel.idx.x;
	int y_new = BestPixel.idx.y;
	Pixel new_pixel = synthesized->pixels[y_new * synWidth + x_new];
	free(EXPPixelArr);
	setPixel(old_pixel, new_pixel);

}

// Assigns the pixel values to the pixels in the window around the center pixel
// Takes in the pixel window, the synthesized image, the width, the height, the synthesized
// image width, the window radius, and the x and y coordinates of the center pixel
void createPixelWindow(Pixel** PixelWindow, Image *synthesized , unsigned int width , unsigned int height, unsigned int synWidth,
						 unsigned int windowRadius, int centerPix_y_coord, int centerPix_x_coord) {

	for(unsigned int i = 0; i < 2*windowRadius+1; i++) {
		for(unsigned int j = 0; j < 2*windowRadius+1; j++) {
			
			// Finds the x and y coordinates of the pixel in the window in relation to the syn image
			int y_coord_pixel = centerPix_y_coord - windowRadius + i;
			int x_coord_pixel = centerPix_x_coord - windowRadius + j;

			// checks if the pixel is in bound
			if( (y_coord_pixel >= 0 && y_coord_pixel < (int)height) && (x_coord_pixel >= 0 && x_coord_pixel < (int)width) ) {
				
				PixelWindow[i*(2*windowRadius+1) + j] = &synthesized->pixels[y_coord_pixel * synWidth + x_coord_pixel];
				
				// if a pixel is unset, we change the pointer in the PixelWindow to a NULL pointer
				if (PixelWindow[i*(2*windowRadius+1) + j]->a == 0){
					PixelWindow[i*(2*windowRadius+1) + j] = NULL;
				}

			}

			// if not, we give it an NULL pointer indicating that it is a pixel not in bound
			else {
				// NULL indicates that pixel is not in the synthesized image range
				PixelWindow[i*(2*windowRadius+1) + j] = NULL;
			}
			
		}

	}
}



// Takes in a double pointer to the window around the TBS pixel,
// a double pointer to the window around the exemplar pixel, and
// the window radius. Finds and returns the gaussian values of 
// the exemplar pixel at the center of its window
double findGaussScore(Pixel** tbsPixelWindow, Pixel** expPixelWindow, int windowRadius){
	double diff = 0.0;

	// setting sigma for Gaussian stdev as in instructions
	double sigma = (2*windowRadius + 1) / 6.4;
	double twoSigmaSquared = 2 * sigma * sigma;
	
	// computing Gaussian difference 
	for(int h = 0; h < 2*windowRadius + 1; h++) {
		for(int k = 0; k < 2*windowRadius + 1; k++) { 

			Pixel* p_tbsPix = tbsPixelWindow[h*(2*windowRadius+1) + k];
			Pixel* p_expPix = expPixelWindow[h*(2*windowRadius+1) + k];

			if(p_tbsPix != NULL && p_expPix != NULL) {

				Pixel tbsPix = *p_tbsPix;
				Pixel expPix = *p_expPix;

				// checks if both pixels are set; BTW the windows being checked have already been stratified such that
				// their only being checked if they all have the same set of pixels
				// TODO: deleted if statement that checks if both are set because isnt that what line 414 already checks?

				int rowOffset = h - windowRadius;
				int colOffset = k - windowRadius;
				double s = exp(-1 * (colOffset * colOffset + rowOffset * rowOffset) / (twoSigmaSquared));
				double d = PixelSquaredDifference(tbsPix, expPix);
				

				diff += (d * s);
			}
				
		}
	}
	
	return diff;
}

// Takes in an array of EXPPixel structs and the size of this array
// Finds and returns the best pixel, a randomly selected pixel
// with a gausian value within 1.1 * the minimum gaussian of all
// pixels.
EXPPixel findBestExemplarPix(EXPPixel* EXPPixelArr, int size) {

	// Finding the minimum Gaussian value in the array
	double minValue = EXPPixelArr->GaussScore;

	for(int i = 0; i < size; i++) {
		double currentGauss = EXPPixelArr[i].GaussScore;
		if(currentGauss <= minValue) {
			minValue = currentGauss;
		}
	}

	// Create array with the indices of pixels in the range
	double adjMin = 1.1 * minValue;

	int indicesInRange[size];
	int counter = 0;

	for(int i = 0; i < size; i++) {
		if(EXPPixelArr[i].GaussScore <= adjMin) {
			indicesInRange[counter] = i;
			counter++;
		}
	}

	// Random value
	int randomIndex = rand() % counter;

	return EXPPixelArr[indicesInRange[randomIndex]];

}








