/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa.c - MPPA interface implementation.
 */

#include <mOS_common_types_c.h>
#include <mOS_vcore_u.h>
#include <mOS_segment_manager_u.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/_default_fcntl.h>
#include <vbsp.h>
#include <utask.h>
#include <HAL/hal/board/boot_args.h>
#include <math.h>
#include <stdlib.h>
#include <mppa_power.h>
#include <mppa_async.h>
#include <mppa_remote.h>
#include <vbsp.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <mppa/osconfig.h>

uint64_t k1_io_read64(unsigned addr)
{
#ifdef BUGGED_MPPA
	return (__k1_io_read64((void *)addr));
#else
	return (0);
#endif
}

int k1_get_cluster_id(void)
{
	return (__k1_get_cluster_id());
}

void k1_dcache_invalidate_mem_area(void *p, size_t n)
{
	__k1_dcache_invalidate_mem_area((__k1_uintptr_t)p, n);
}
