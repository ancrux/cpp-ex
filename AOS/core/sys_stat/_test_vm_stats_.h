
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_vm_stats_test(int argc, ACE_TCHAR* argv[])
{	
	int rc = 0;

	sg_mem_stats *mem_stats;
	sg_swap_stats *swap_stats;

	long long total, free;

	/* Initialise statgrab */
	sg_init();

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	if( ((mem_stats=sg_get_mem_stats()) != NULL) && (swap_stats=sg_get_swap_stats()) != NULL){
		printf("Total memory in bytes : %lld\n", mem_stats->total);
		printf("Used memory in bytes : %lld\n", mem_stats->used);
		printf("Cache memory in bytes : %lld\n", mem_stats->cache);
		printf("Free memory in bytes : %lld\n", mem_stats->free);

		printf("Swap total in bytes : %lld\n", swap_stats->total);
		printf("Swap used in bytes : %lld\n", swap_stats->used);	
		printf("Swap free in bytes : %lld\n", swap_stats->free);

		total = mem_stats->total + swap_stats->total;
		free = mem_stats->free + swap_stats->free;

		printf("Total VM usage : %5.2f%%\n", 100 - (((float)total/(float)free)));

	}
	else {
		printf("Unable to get VM stats: %s\n", sg_str_error(sg_get_error()));
		exit(1);
	}

	return rc;
}
