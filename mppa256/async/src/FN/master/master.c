#include <async_util.h>
#include <spawn_util.h>
#include <stdint.h> // Necessary for uint64_t.
#include <stdlib.h> // Necessary for malloc().
#include <stddef.h> // Necessary for size_t.

#include "master.h"

int friendly_numbers(int start_num, int end_num) {
	async_master_init();

	spawn_slaves();

	async_master_start();

	join_slaves();

	async_master_finalize();
}