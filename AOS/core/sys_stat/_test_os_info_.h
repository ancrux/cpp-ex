
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_os_info_test(int argc, ACE_TCHAR* argv[])
{	
	int rc = 0;

	sg_host_info *general_stats;

	/* Initialise statgrab */
	sg_init();

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	general_stats = sg_get_host_info();

	if(general_stats == NULL){
		fprintf(stderr, "Failed to get os stats\n");
		exit(1);
	}

	printf("OS name : %s\n", general_stats->os_name);
	printf("OS release : %s\n", general_stats->os_release);
	printf("OS version : %s\n", general_stats->os_version);
	printf("Hardware platform : %s\n", general_stats->platform);
	printf("Machine nodename : %s\n", general_stats->hostname);
	printf("Machine uptime : %lld\n", (long long)general_stats->uptime);

	return rc;
}
