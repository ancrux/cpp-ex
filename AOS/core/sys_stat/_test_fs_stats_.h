
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_fs_stats_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	/* We default to 1 second updates and displaying in bytes*/
	//int delay = 1;
	//char units = 'b';
	//char units = 'k';
	//char units = 'm';

	sg_fs_stats *fs_stats;
	int num_fs_stats;

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
	//delay = delay * 1000;
#endif

	/* Initialise statgrab */
	sg_init();

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	fs_stats = sg_get_fs_stats(&num_fs_stats);
	if ( fs_stats )
	{
		for(int x = 0; x < num_fs_stats; x++)
		{
			//device_name  
			//The name known to the operating system. (eg. on linux it might be hda)
			printf("device_name:%s\n", fs_stats->device_name);
			
			//fs_type  
			//The type of the filesystem.
			printf("fs_type:%s\n", fs_stats->fs_type);

			//mnt_point  
			//The mount point of the file system.
			printf("mnt_point:%s\n", fs_stats->mnt_point);

			//size  
			//The size, in bytes, of the file system.
			printf("size:%lld\n", fs_stats->size);

			//used  
			//The amount of space, in bytes, used on the filesystem.
			printf("used:%lld\n", fs_stats->used);

			//avail  
			//The amount of space, in bytes, available on the filesystem.
			printf("avail:%lld\n", fs_stats->avail);

			//total_inodes  
			//The total number of inodes in the filesystem.
			printf("total_inodes:%lld\n", fs_stats->total_inodes);

			//used_inodes  
			//The number of used inodes in the filesystem.
			printf("used_inodes:%lld\n", fs_stats->used_inodes);

			//free_inodes  
			//The number of free inodes in the filesystem.
			printf("free_inodes:%lld\n", fs_stats->free_inodes);

			//avail_inodes  
			//The number of free inodes available to non\-privileged processes.
			printf("avail_inodes:%lld\n", fs_stats->avail_inodes);

			//io_size  
			//A suggested optimal block size for IO operations \-\- if you're reading or writing lots of data, do it in chunks of this size.
			printf("io_size:%lld\n", fs_stats->io_size);

			//block_size  
			//How big blocks actually are on the underlying filesystem (typically for purposes of stats reporting).
			printf("block_size:%lld\n", fs_stats->block_size);

			//total_blocks  
			//The total number of blocks in the filesystem.
			printf("total_blocks:%lld\n", fs_stats->total_blocks);

			//free_blocks  
			//The number of free blocks in the filesystem.
			printf("free_blocks:%lld\n", fs_stats->free_blocks);

			//used_blocks  
			//The number of used blocks in the filesystem.
			printf("used_blocks:%lld\n", fs_stats->used_blocks);

			//avail_blocks  
			//The number of free blocks available to non\-privileged processes.
			printf("avail_blocks:%lld\n", fs_stats->avail_blocks);

			fs_stats++;

			printf("\n");
		}
	}

	return rc;

}
