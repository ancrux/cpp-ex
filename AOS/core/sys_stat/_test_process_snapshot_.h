
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_process_snapshot_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	sg_process_stats *ps;
	int ps_size;
	int x;
	char *state = NULL;

	/* Initialise statgrab */
	sg_init();

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	ps = sg_get_process_stats(&ps_size);

	if(ps == NULL){
		fprintf(stderr, "Failed to get process snapshot\n");
		return 1;
	}

	qsort(ps, ps_size, sizeof *ps, sg_process_compare_pid);

	printf("%5s %5s %5s %5s %5s %5s %5s %6s %6s %9s %-10s %-4s %-8s %-20s %s\n",
	 	"pid", "ppid", "pgid", "uid", "euid", "gid", "egid", "size", "res", "time", "cpu", "nice", "state", "name", "title");

	for(x=0;x<ps_size;x++){
		switch (ps->state) {
		case SG_PROCESS_STATE_RUNNING:
			state = "RUNNING";
			break;
		case SG_PROCESS_STATE_SLEEPING:
			state = "SLEEPING";
			break;
		case SG_PROCESS_STATE_STOPPED:
			state = "STOPPED";
			break;
		case SG_PROCESS_STATE_ZOMBIE:
			state = "ZOMBIE";
			break;
		case SG_PROCESS_STATE_UNKNOWN:
		default:
			state = "UNKNOWN";
			break;
		}
		printf("%5d %5d %5d %5d %5d %5d %5d %5dM %5dM %8ds %10f %4d %-8s %-20s %s\n",
			(int)ps->pid, (int)ps->parent, (int)ps->pgid, (int)ps->uid,
			(int)ps->euid, (int)ps->gid, (int)ps->egid,
			(int)(ps->proc_size / (1024*1024)),
			(int)(ps->proc_resident / (1024*1024)),
			(int)ps->time_spent, (float)ps->cpu_percent,
			(int)ps->nice, state,
			ps->process_name, ps->proctitle);
		ps++;
	}

	return rc;
}
