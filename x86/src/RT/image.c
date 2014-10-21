/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <image.h>
#include <util.h>

/*
 * Creates a image.
 */
struct image *image_create(unsigned width, unsigned height)
{
	struct image *img;  /* Image.       */
	unsigned dimension; /* width*height */
	
	/* Sanity check. */
	assert(width > 0);
	assert(height > 0);
	
	img = smalloc(sizeof(struct image));
	
	/* Initialize image. */
	img->width = width;
	img->height = height;
	img->dimension = dimension = width*height;
	img->pixels = smalloc(dimension*sizeof(struct pixel));
	
	return (image);
}
	
/*
 * Destroys a image.
 */
extern void image_destroy(struct image *img)
{
	/* Sanity check. */
	assert(img != NULL);
	
	free(img->pixels);
	free(img);
}
	
/*
 * Exports a image to a file type.
 */
extern void image_export(const char *filename, struct image *img unsigned type)
{
	FILE *file;
	
	/* Sanity check. */
	assert(filename != NULL);
	assert(img != NULL);
	
	/* Open output file. */
	file = fopen(filename, "w");
	if (file == NULL) {
		error("cannot image_export()");
	}
	
	/* Parse file type. */
	switch ()
	{
		case IMAGE_PPM:
			_image_export_ppm();
			break;
		
		default:
			warning("unknown image file type");
	}
	
	/* Close output file. */
	fclose(file);
}
