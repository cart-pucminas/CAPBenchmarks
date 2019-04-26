#ifndef _KERNEL_H_
#define _KERNEL_H_

	/* Maximum thread num per cluster. */
	#define MAX_THREADS (16)
	
	/* Maximum chunk size. */
	#define CHUNK_SIZE (512)
	
	/* Chunk size * Chunk size. */
	#define CHUNK_SIZE_SQRD (262144)
	
	/* Maximum mask size. */
	#define MASK_SIZE (54)
	
	/* Mask radius. */
	#define MASK_RADIUS (3)
	
	/* Maximum image size. */
	#define IMG_SIZE (24576)
	
	/* Threshold value between central pixel and neighboor pixel. */
	#define THRESHOLD (20)
	
	/* Type of messages. */
	#define MSG_CHUNK 2
	#define MSG_DIE   1

#endif /* _KERNEL_H_ */