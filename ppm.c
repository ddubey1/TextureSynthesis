#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "ppm.h"
#include "image.h"


/* helper function for read_ppm, takes a filehandle and 
* reads off at most the maxWhiteSpace white-space characters, returning
* the number of white-space characters actually read
*/
int ppm_read_whitespace( FILE *fp , int maxWhiteSpace )
{
	int wsCount = 0;
	for( ; maxWhiteSpace==-1 || wsCount<maxWhiteSpace ; wsCount++ )
	{
		int ch;
		if( !isspace( ch=fgetc(fp) ) )
		{
			ungetc( ch , fp );
			return wsCount;
		}
	}
	return wsCount;
}

/* helper function for read_ppm, takes a filehandle and 
* and reads a number, but detects and skips comment lines
*/
int ppm_read_num( FILE *fp )
{
	assert( fp );

	int ch;
	while( ( ch=fgetc(fp) )=='#' ) // # marks a comment line
	{
		while( ( ( ch=fgetc(fp) )!='\n' ) && ch!=EOF )
		{
			/* discard characters til end of line */
		}
	}
	ungetc( ch , fp ); // put back the last thing we found

	int val;
	if( fscanf( fp , "%d" , &val )==1 ) // try to get an int
	{
		return val; // we got a value, so return it
	}
	else
	{
		fprintf( stderr, "[ERROR] ppm_read_num: - failed to read number from file\n" );
		return -1;
	}
}




Image *ReadPPM( FILE *fp )
{
	/* confirm that we received a good file handle */
	assert( fp!=NULL );

	/* declare an image (but do not allocate -- yet) */
	Image *img = NULL;

	/* read in tag; fail if not P6 */
	char tag[20];
	tag[19] = '\0';
	fscanf( fp , "%19s\n" , tag );
	if( strncmp( tag , "P6" , 20 ) )
	{
		fprintf( stderr , "[ERROR] ReadPPM: Not a PPM (bad tag)\n" );
		return img;
	}


	/* read image dimensions */
	// NOTE: cols, then rows (i.e. X size followed by Y size)

	int width = ppm_read_num( fp );
	ppm_read_whitespace( fp , -1 );
	int height = ppm_read_num( fp );
	ppm_read_whitespace( fp , -1 );

	// confirm that dimensions are positive
	if( width<=0 || height<=0 )
	{
		fprintf( stderr, "[ERROR] ReadPPM: PPM file with non-positive dimensions: %d x %d\n" , width , height );
		return img;
	}

	// allocate the image (and its pixels)
	img = AllocateImage( width , height );
	if( !img )
	{
		fprintf( stderr , "[ERROR] ReadPPM: Failed to allocate image: %d x %d\n" , width , height );
		return img;
	}

	// fead in colors; fail if not 255
	int colors = ppm_read_num(fp);
	if( colors!=255 )
	{
		fprintf( stderr , "[ERROR] ReadPPM: PPM file with colors different from 255\n" );
		FreeImage( &img );
		return img;
	}
	if( ppm_read_whitespace( fp , 1 )!=1 )
	{
		fprintf( stderr , "[ERROR] ReadPPM: PPM failed to read white-space after max color\n" );
		FreeImage( &img );
		return img;
	}

	/* finally, read in Pixels */
	/* read in the binary Pixel data */
	for( unsigned int i=0 ; i<(unsigned int)( width*height ) ; i++ )
	{
		if( fread( img->pixels+i , sizeof(unsigned char) , 3 , fp )!=3 )
		{
			fprintf( stderr, "[ERROR] PPMRead: failed to read pixel from file: %d / %d\n" , i , width*height );
			FreeImage( &img );
			return img;
		}
		img->pixels[i].a = 255;
	}

	// Return the image struct pointer
	return img;
}



/* Write given image to disk as a PPM.
* Return -1 if any failure occurs, otherwise return the number of pixels written.
*/
int WritePPM( FILE *fp , const Image *img )
{
	// Checks if ppm file is valid
	if (fp == NULL){
		return -1;
	}

	// Specifying PPM image tag
	fprintf(fp, "P6\n %d %d\n 255\n", img->width, img->height);

	// Writing pixels
	int pixels_written = 0;

	for( unsigned int i=0 ; i<(unsigned int)( (img->width)*(img->height) ) ; i++ ) {
		fwrite( img->pixels+i , sizeof(unsigned char) , 3 , fp );
		pixels_written++;
	}

	return pixels_written;

}