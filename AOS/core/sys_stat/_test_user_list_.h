
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_user_list_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	//int delay = 1;
	sg_user_stats *users;

	//extern char *optarg;
	//int c;
	//while ((c = getopt(argc, argv, "d:")) != -1){
	//	switch (c){
	//		case 'd':
	//			delay = atoi(optarg);
	//			break;
	//	}
	//}

	/* Initialise statgrab */
	sg_init();

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	if( (users = sg_get_user_stats()) != NULL){
		printf("Users : %s\n", users->name_list);
		printf("Number of users : %d\n", users->num_entries);
	}

	return rc;
}
