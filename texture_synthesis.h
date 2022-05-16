#ifndef TEXTURE_SYNTHESIS_INCLUDED
#define TEXTURE_SYNTHESIS_INCLUDED
#include "image.h"

/** A struct storing information about a to-be-synthesized pixel*/
typedef struct
{
	/** The index of the pixel to be synthesized*/
	PixelIndex idx;
	
	/** The number of neighboring pixels in a set state*/
	unsigned int neighborCount;
	
	/** A member for storing a value used to resolve ties*/
	unsigned int r;
	
} TBSPixel;

/** A struct storing information about an exemplar pixel under consideration for 
 * being set as the value of the tbs pixel.
*/
typedef struct
{
	// index of exemplar pixel
	PixelIndex idx;

	// gaussian value (as calculated)
	double GaussScore;

} EXPPixel;

/** A function that compares two TBSPixels and returns a negative number if the first should come earlier in the sort order and a positive number if it should come later*/
int CompareTBSPixels( const void *v1 , const void *v2 );

/** A function that sorts an array of TBSPixels*/
int SortTBSPixels( TBSPixel *tbsPixels , unsigned int sz );

/** A function that extends the exemplar into an image with the specified dimensions, using the prescribed window radius -- the verbose argument is passed in to enable logging to the command prompt, if desired*/
Image *SynthesizeFromExemplar( const Image *exemplar , unsigned int outWidth , unsigned int outHeight , unsigned int windowRadius , bool verbose );

/** A helper function that changes color of pixels from old color to new color */
void setPixel(Pixel * old_color, const Pixel new_color);

/** A helper function that checks if the inputted index is in bound */
int checkIndexInBound(int index, unsigned int Width, unsigned int Height);

/** A helper function that finds the number of set, existing neigboring pixels */
int findNumNeigbhors(Image *synthesized, int index, unsigned int Width, unsigned int Height);

/** A helper function that finds all TBS Pixels */
TBSPixel *findTBSPixel(Image *synthesized, unsigned int Width , unsigned int Height, int* size);

/** A function that synthesizes all Pixels in the given image */
void synthesizeTexture(Image *synthesized , unsigned int exWidth , unsigned int exHeight , 
						unsigned int synWidth, unsigned int synHeight, unsigned int windowRadius);

/** A helper function to set the value of the to-be-set pixel with the greatest amount of neighbors*/
void synthesizePixel(TBSPixel* TBSPixelArr, Image *synthesized , unsigned int exWidth , unsigned int exHeight , 
						unsigned int synWidth, unsigned int synHeight, unsigned int windowRadius);

/** A helper function to compute the Gaussian difference of a expPixelWindow compared to a tbsPixelWindow*/
double findGaussScore(Pixel** tbsPixelWindow, Pixel** expPixelWindow, int radius);

/** A function that finds the pixel with the lowest Gaussian value in an array of exemplar pixels */
EXPPixel findBestExemplarPix(EXPPixel* EXPPixelArr, int size);

/** function that takes a pixel window and assigns the pixel values around the center of the window */
void createPixelWindow(Pixel** PixelWindow, Image *synthesized , unsigned int width , unsigned int height, unsigned int synWidth,
						 unsigned int windowRadius, int centerPix_y_coord, int centerPix_x_coord);

#endif // TEXTURE_SYNTHESIS_INCLUDED