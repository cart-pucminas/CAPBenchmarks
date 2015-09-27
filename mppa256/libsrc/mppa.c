/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa.c - MPPA interface implementation.
 */

#include <stdint.h>
#include <mppa/osconfig.h>

uint64_t k1_io_read64(unsigned addr)
{
	return (__k1_io_read64((void *)addr));
}

int k1_get_cluster_id(void)
{
	return (__k1_get_cluster_id());
}

int k1_dcache_invalidate_mem_area(void *p, size_t n)
{
	return (__k1_dcache_invalidate_mem_area((__k1_uintptr_t)p, n));
}
