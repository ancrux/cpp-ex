
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_network_iface_stats_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	sg_network_iface_stats *network_iface_stats;
	int iface_count, i;

	/* Initialise statgrab */
	sg_init();

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	network_iface_stats = sg_get_network_iface_stats(&iface_count);

	if(network_iface_stats == NULL){
		fprintf(stderr, "Failed to get network interface stats\n");
		exit(1);
	}

	if (argc != 1) {
		/* If an argument is given, use bsearch to find just that
		 * interface. */
		sg_network_iface_stats key;

		key.interface_name = argv[1];
		network_iface_stats = (sg_network_iface_stats *) bsearch(&key, network_iface_stats,
		                              iface_count,
		                              sizeof *network_iface_stats,
		                              sg_network_iface_compare_name);
		if (network_iface_stats == NULL) {
			fprintf(stderr, "Interface %s not found\n", argv[1]);
			exit(1);
		}
		iface_count = 1;
	}

	printf("Name\tSpeed\tDuplex\n");
	for(i = 0; i < iface_count; i++) {
		printf("%s\t%d\t", network_iface_stats->interface_name, network_iface_stats->speed);
		switch (network_iface_stats->duplex) {
		case SG_IFACE_DUPLEX_FULL:
			printf("full\n");
			break;
		case SG_IFACE_DUPLEX_HALF:
			printf("half\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
		network_iface_stats++;
	}

	return rc;
}
