
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_network_traffic_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	/* We default to 1 second updates and displaying in bytes*/
	int delay = 1;
	char units = 'b';
	//char units = 'k';
	//char units = 'm';

	sg_network_io_stats *network_stats;
	int num_network_stats;

	/* Parse command line options */
	//extern char *optarg;
	//int c;
	//while ((c = getopt(argc, argv, "d:bkm")) != -1){
	//	switch (c){
	//		case 'd':
	//			delay =	atoi(optarg);
	//			break;
	//		case 'b':
	//			units = 'b';
	//			break;
	//		case 'k':
	//			units = 'k';	
	//			break;
	//		case 'm':
	//			units = 'm';
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

	/* We are not interested in the amount of traffic ever transmitted, just differences. 
	 * Because of this, we do nothing for the very first call.
	 */

	network_stats = sg_get_network_io_stats_diff(&num_network_stats);
	if (network_stats == NULL){
		perror("Error. Failed to get network stats");
		return 1;
	}

	/* Clear the screen ready for display the network stats */
	//printf("\033[2J");

	/* Keep getting the network stats */
	int n_loop = 0;
	while ( (network_stats = sg_get_network_io_stats_diff(&num_network_stats)) != NULL){
		++n_loop;
		if ( n_loop > 5 )
			break;		
		
		int x;
		int line_number = 2;

		long long total_tx=0;
		long long total_rx=0;
		long long total_ipackets=0;
		long long total_opackets=0;
		long long total_ierrors=0;
		long long total_oerrors=0;
		long long total_collisions=0;

		for(x = 0; x < num_network_stats; x++){	
			/* Print at location 2, linenumber the interface name */
			//printf("\033[%d;2H%-30s : %-10s", line_number++, "Network Interface Name", network_stats->interface_name);
			printf("%d:%-30s : %-10s\n", line_number++, "Network Interface Name", network_stats->interface_name);
			/* Print out at the correct location the traffic in the requsted units passed at command time */
			switch(units){
				case 'b':
					//printf("\033[%d;2H%-30s : %8lld b", line_number++, "Network Interface Rx", network_stats->rx);
					//printf("\033[%d;2H%-30s : %8lld b", line_number++, "Network Interface Tx", network_stats->tx);
					printf("%d:%-30s : %8lld b\n", line_number++, "Network Interface Rx", network_stats->rx);
					printf("%d:%-30s : %8lld b\n", line_number++, "Network Interface Tx", network_stats->tx);
					break;
				case 'k':
					//printf("\033[%d;2H%-30s : %5lld k", line_number++, "Network Interface Rx", (network_stats->rx / 1024));
					//printf("\033[%d;2H%-30s : %5lld", line_number++, "Network Interface Tx", (network_stats->tx / 1024));
					printf("%d:%-30s : %5lld k\n", line_number++, "Network Interface Rx", (network_stats->rx / 1024));
					printf("%d:%-30s : %5lld k\n", line_number++, "Network Interface Tx", (network_stats->tx / 1024));
					break;
				case 'm':
					//printf("\033[%d;2H%-30s : %5.2f m", line_number++, "Network Interface Rx", network_stats->rx / (1024.00*1024.00));
					//printf("\033[%d;2H%-30s : %5.2f m", line_number++, "Network Interface Tx", network_stats->tx / (1024.00*1024.00));
					printf("%d:%-30s : %5.2f m\n", line_number++, "Network Interface Rx", network_stats->rx / (1024.00*1024.00));
					printf("%d:%-30s : %5.2f m\n", line_number++, "Network Interface Tx", network_stats->tx / (1024.00*1024.00));
			}
			//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Interface packets in", network_stats->ipackets);
			//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Interface packets out", network_stats->opackets);
			//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Interface errors in", network_stats->ierrors);
			//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Interface errors out", network_stats->oerrors);
			//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Interface collisions", network_stats->collisions);
			//printf("\033[%d;2H%-30s : %ld ", line_number++, "Network Interface systime", (long) network_stats->systime);
			printf("%d:%-30s : %lld \n", line_number++, "Network Interface packets in", network_stats->ipackets);
			printf("%d:%-30s : %lld \n", line_number++, "Network Interface packets out", network_stats->opackets);
			printf("%d:%-30s : %lld \n", line_number++, "Network Interface errors in", network_stats->ierrors);
			printf("%d:%-30s : %lld \n", line_number++, "Network Interface errors out", network_stats->oerrors);
			printf("%d:%-30s : %lld \n", line_number++, "Network Interface collisions", network_stats->collisions);
			printf("%d:%-30s : %lld \n", line_number++, "Network Interface systime", (long long) network_stats->systime);

			/* Add a blank line between interfaces */	
			line_number++;

			/* Add up this interface to the total so we can display a "total" network io" */
			total_tx+=network_stats->tx;
			total_rx+=network_stats->rx;
			total_ipackets+=network_stats->ipackets;
			total_opackets+=network_stats->opackets;
			total_ierrors+=network_stats->ierrors;
			total_oerrors+=network_stats->oerrors;
			total_collisions+=network_stats->collisions;

			/* Move the pointer onto the next interface. Since this returns a static buffer, we dont need
			 * to keep track of the orginal pointer to free later */
			network_stats++;
		}

		//printf("\033[%d;2H%-30s : %-10s", line_number++, "Network Interface Name", "Total Network IO");
		printf("%d:%-30s : %-10s\n", line_number++, "Network Interface Name", "Total Network IO");
		switch(units){
			case 'b':
				//printf("\033[%d;2H%-30s : %8lld b", line_number++, "Network Total Rx", total_rx);
				//printf("\033[%d;2H%-30s : %8lld b", line_number++, "Network Total Tx", total_tx);
				printf("%d:%-30s : %8lld b\n", line_number++, "Network Total Rx", total_rx);
				printf("%d:%-30s : %8lld b\n", line_number++, "Network Total Tx", total_tx);
				break;
			case 'k':
				//printf("\033[%d;2H%-30s : %5lld k", line_number++, "Network Total Rx", (total_rx / 1024));
				//printf("\033[%d;2H%-30s : %5lld k", line_number++, "Network Total Tx", (total_tx / 1024));
				printf("%d:%-30s : %5lld k\n", line_number++, "Network Total Rx", (total_rx / 1024));
				printf("%d:%-30s : %5lld k\n", line_number++, "Network Total Tx", (total_tx / 1024));
				break;
			case 'm':
				//printf("\033[%d;2H%-30s : %5.2f m", line_number++, "Network Total Rx", (total_rx  / (1024.00*1024.00)));
				//printf("\033[%d;2H%-30s : %5.2f m", line_number++, "Network Total Tx", (total_tx  / (1024.00*1024.00)));
				printf("%d:%-30s : %5.2f m\n", line_number++, "Network Total Rx", (total_rx  / (1024.00*1024.00)));
				printf("%d:%-30s : %5.2f m\n", line_number++, "Network Total Tx", (total_tx  / (1024.00*1024.00)));
				break;
		}
		//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Total packets in", total_ipackets);
		//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Total packets out", total_opackets);
		//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Total errors in", total_ierrors);
		//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Total errors out", total_oerrors);
		//printf("\033[%d;2H%-30s : %lld ", line_number++, "Network Total collisions", total_collisions);

		printf("%d:%-30s : %lld \n", line_number++, "Network Total packets in", total_ipackets);
		printf("%d:%-30s : %lld \n", line_number++, "Network Total packets out", total_opackets);
		printf("%d:%-30s : %lld \n", line_number++, "Network Total errors in", total_ierrors);
		printf("%d:%-30s : %lld \n", line_number++, "Network Total errors out", total_oerrors);
		printf("%d:%-30s : %lld \n", line_number++, "Network Total collisions", total_collisions);

		fflush(stdout);

#ifdef WIN32
		::Sleep(delay);
#else
		sleep(delay);
#endif

	}

	return rc;

}
