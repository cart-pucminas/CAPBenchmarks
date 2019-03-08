#ifndef _KERNEL_H_
#define _KERNEL_H_

	#define MICRO (1.0/1000000)

	#define CHUNK_SIZE (512)     /* Maximum chunk size. */
	#define MASK_SIZE   15
    #define NCHUNKS    1024     /* Maximum number of chunks */

	#define PI 3.14159265359    /* pi */
	#define E 2.71828182845904  /* e */
	#define SD 0.8              /* Standard deviation. */

	/* Type of messages. */
	#define MSG_CHUNK 2
	#define MSG_DIE   1

	/* RMEM definitions. */
	#define SIZE_MASKSIZE    (sizeof(int))
	#define SIZE_IMGSIZE     (sizeof(int))

	#define SIZE_MASK        (masksize * masksize * sizeof(double))
	#define SIZE_IMAGE       (imgsize * imgsize * sizeof(unsigned char))
	#define SIZE_NEWIMAGE    SIZE_IMAGE
    #define SIZE_CHUNKS      NCHUNKS * (CHUNK_SIZE+masksize-1) * (CHUNK_SIZE+masksize-1)

	#define OFF_MASKSIZE   (0)
	#define OFF_IMGSIZE    (OFF_MASKSIZE  + SIZE_MASKSIZE)
	#define OFF_MASK       (OFF_IMGSIZE   + SIZE_IMGSIZE) 
	#define OFF_IMAGE      (OFF_MASK      + SIZE_MASK)
    #define OFF_NEWIMAGE   (OFF_IMAGE     + SIZE_IMAGE)
	#define OFF_CHUNKS     (OFF_NEWIMAGE  + SIZE_NEWIMAGE)
    #define OFF_CHUNKSIZE  (OFF_CHUNKS    + SIZE_CHUNKS)

	#define MASK(i, j) \
		mask[(i)*masksize + (j)]

	#define CHUNK(i, j) \
		chunk[(i)*(CHUNK_SIZE + masksize - 1) + (j)]

	#define NEWCHUNK(i, j) \
		newchunk[(i)*CHUNK_SIZE + (j)]

	#define CHUNK_SIZE_SQRD (262144)

#endif /* _KERNEL_H_ */
