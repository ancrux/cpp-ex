
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_disk_traffic_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	/* We default to 1 second updates and displaying in bytes*/
	int delay = 1;
	char units = 'b';
	//char units = 'k';
	//char units = 'm';

	sg_disk_io_stats *diskio_stats;
	int num_diskio_stats;

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

	diskio_stats = sg_get_disk_io_stats_diff(&num_diskio_stats);
	if (diskio_stats == NULL){
		perror("Error. Failed to get disk stats");
		return 1;
	}

	/* Clear the screen ready for display the disk stats */
	//printf("\033[2J");

	/* Keep getting the disk stats */
	int n_loop = 0;
	while ( (diskio_stats = sg_get_disk_io_stats_diff(&num_diskio_stats)) != NULL){
		++n_loop;
		if ( n_loop > 5 )
			break;

		int x;
		int line_number = 2;

		long long total_write=0;
		long long total_read=0;

		for(x = 0; x < num_diskio_stats; x++){	
			/* Print at location 2, linenumber the interface name */
			//printf("\033[%d;2H%-25s : %-10s", line_number++, "Disk Name", diskio_stats->disk_name);
			printf("%d:%-25s : %-10s\n", line_number++, "Disk Name", diskio_stats->disk_name);
			/* Print out at the correct location the traffic in the requsted units passed at command time */
			switch(units){
				case 'b':
					//printf("\033[%d;2H%-25s : %8lld b", line_number++, "Disk read", diskio_stats->read_bytes);
					//printf("\033[%d;2H%-25s : %8lld b", line_number++, "Disk write", diskio_stats->write_bytes);
					printf("%d:%-25s : %8lld b\n", line_number++, "Disk read", diskio_stats->read_bytes);
					printf("%d:%-25s : %8lld b\n", line_number++, "Disk write", diskio_stats->write_bytes);
					break;
				case 'k':
					//printf("\033[%d;2H%-25s : %5lld k", line_number++, "Disk read", (diskio_stats->read_bytes / 1024));
					//printf("\033[%d;2H%-25s : %5lld", line_number++, "Disk write", (diskio_stats->write_bytes / 1024));
					printf("%d:%-25s : %5lld k\n", line_number++, "Disk read", (diskio_stats->read_bytes / 1024));
					printf("%d:%-25s : %5lld k\n", line_number++, "Disk write", (diskio_stats->write_bytes / 1024));
					break;
				case 'm':
					//printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Disk read", diskio_stats->read_bytes / (1024.00*1024.00));
					//printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Disk write", diskio_stats->write_bytes / (1024.00*1024.00));
					printf("%d:%-25s : %5.2f m\n", line_number++, "Disk read", diskio_stats->read_bytes / (1024.00*1024.00));
					printf("%d:%-25s : %5.2f m\n", line_number++, "Disk write", diskio_stats->write_bytes / (1024.00*1024.00));
			}
			//printf("\033[%d;2H%-25s : %ld ", line_number++, "Disk systime", (long) diskio_stats->systime);
			printf("%d:%-25s : %ld \n", line_number++, "Disk systime", (long) diskio_stats->systime);

			/* Add a blank line between interfaces */	
			line_number++;

			/* Add up this interface to the total so we can display a "total" disk io" */
			total_write+=diskio_stats->write_bytes;
			total_read+=diskio_stats->read_bytes;

			/* Move the pointer onto the next interface. Since this returns a static buffer, we dont need
			 * to keep track of the orginal pointer to free later */
			diskio_stats++;
		}

		//printf("\033[%d;2H%-25s : %-10s", line_number++, "Disk Name", "Total Disk IO");
		printf("%d:%-25s : %-10s\n", line_number++, "Disk Name", "Total Disk IO");
		switch(units){
			case 'b':
				//printf("\033[%d;2H%-25s : %8lld b", line_number++, "Disk Total read", total_read);
				//printf("\033[%d;2H%-25s : %8lld b", line_number++, "Disk Total write", total_write);
				printf("%d:%-25s : %8lld b\n", line_number++, "Disk Total read", total_read);
				printf("%d:%-25s : %8lld b\n", line_number++, "Disk Total write", total_write);
				break;
			case 'k':
				//printf("\033[%d;2H%-25s : %5lld k", line_number++, "Disk Total read", (total_read / 1024));
				//printf("\033[%d;2H%-25s : %5lld k", line_number++, "Disk Total write", (total_write / 1024));
				printf("%d:%-25s : %5lld k\n", line_number++, "Disk Total read", (total_read / 1024));
				printf("%d:%-25s : %5lld k\n", line_number++, "Disk Total write", (total_write / 1024));
				break;
			case 'm':
				//printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Disk Total read", (total_read  / (1024.00*1024.00)));
				//printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Disk Total write", (total_write  / (1024.00*1024.00)));
				printf("%d:%-25s : %5.2f m\n", line_number++, "Disk Total read", (total_read  / (1024.00*1024.00)));
				printf("%d:%-25s : %5.2f m\n", line_number++, "Disk Total write", (total_write  / (1024.00*1024.00)));
				break;
		}
		printf("\n");

		fflush(stdout);

#ifdef WIN32
		::Sleep(delay);
#else
		sleep(delay);
#endif

	}

	return rc;

}
