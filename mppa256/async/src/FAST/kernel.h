#ifndef _KERNEL_H_
#define _KERNEL_H_

	/* Maximum thread num per cluster. */
	#define MAX_THREADS (16)
	
	/* Maximum chunk size. */
	#define CHUNK_SIZE (512)
	
	/* Maximum mask size. */
	#define MASK_SIZE (54)
	
	/* Mask radius. */
	#define MASK_RADIUS (3)
	
	/* Maximum image size. */
	#define IMG_SIZE (24576)
	
	/* Threshold value between central pixel and neighboor pixel. */
	#define THRESHOLD (20)
	
	/* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0

#endif /* _KERNEL_H_ */