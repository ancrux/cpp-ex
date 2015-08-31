
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_page_stats_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	int delay = 1;
	sg_page_stats *page_stats;

	//extern char *optarg;
	//int c;
	//while ((c = getopt(argc, argv, "d:")) != -1){
	//	switch (c){
	//		case 'd':
	//			delay = atoi(optarg);
	//			break;
	//	}
	//}

#ifdef WIN32
	delay = delay * 1000;
#endif

	/* Initialise statgrab */
	sg_init();

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	int n_loop = 0;
	while( (page_stats = sg_get_page_stats_diff()) != NULL){
		++n_loop;
		if ( n_loop > 5 )
			break;

		printf("Pages in : %lld\n", page_stats->pages_pagein);
		printf("Pages out : %lld\n", page_stats->pages_pageout);
#ifdef WIN32
		::Sleep(delay);
#else
		sleep(delay);
#endif
	}

	return rc;
}
