/* -*- mode: C; c-file-style: "k&r"; -*-
 *---------------------------------------------------------------------------*
 *
 * Copyright (c) 2000, Johan Bengtsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *---------------------------------------------------------------------------*/

#define _USE_BSD

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include <errno.h>

#include "machdep.h"

#define CAN_USE_RLIMIT_RSS
#define CAN_USE_RLIMIT_CPU

int main(int argc, char *argv[])
{
	struct rusage kid_usage;
	pid_t kid;
	int kid_status;
	int i, opt, echo_args = 0, exit_flag;

	int maxkbytes = 0;	// kilobytes
	int maxseconds = 0; // seconds
	long int maxmillis = 0;

	long int max_vsize = 0;
	unsigned int start, end;

	struct memtime_info info;

	if (argc < 2)
	{
		char *tmp = strrchr(argv[0], '/');
		tmp = (tmp ? tmp + 1 : argv[0]);

		fprintf(stderr,
				"%s: usage %s [-e] [-m <maxkilobytes>] [-c <maxcpuseconds>] <cmd> [<params>]\n",
				tmp, tmp);
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "+et:m:c:")) != -1)
	{

		switch (opt)
		{
		case 'e':
			echo_args = 1;
			break;

		case 'm':
			errno = 0;
			maxkbytes = atoi(optarg);
			if (errno)
			{
				perror("Illegal argument to m option");
				exit(EXIT_FAILURE);
			}
			break;

		case 'c':
			errno = 0;
			maxseconds = atoi(optarg);
			if (errno)
			{
				perror("Illegal argument to c option");
				exit(EXIT_FAILURE);
			}
			maxmillis = 1000 * maxseconds;
			break;
		}
	}

	if (echo_args)
	{
		fprintf(stderr, "Command line: ");
		for (i = optind; i < argc; i++)
			fprintf(stderr, "%s ", argv[i]);
		fprintf(stderr, "\n");
	}

	start = get_time();

	switch (kid = fork())
	{

	case -1:
		perror("fork failed");
		exit(EXIT_FAILURE);

	case 0:

		if (maxkbytes > 0)
		{
			if (set_mem_limit((long int)maxkbytes * 1024))
			{
				printf("error\n");
				;
			}
		}

		if (maxseconds > 0)
		{
			set_cpu_limit((long int)maxseconds);
		}

		execvp(argv[optind], &(argv[optind]));
		perror("exec failed");
		exit(EXIT_FAILURE);

	default:
		break;
	}

	if (!init_machdep(kid))
	{
		fprintf(stderr, "%s: Failed to initialise sampling.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	do
	{

		get_sample(&info);

		max_vsize = (info.vsize_kb > max_vsize ? info.vsize_kb : max_vsize);

		usleep(100000);

		exit_flag = ((wait4(kid, &kid_status, WNOHANG, &kid_usage) == kid) && (WIFEXITED(kid_status) || WIFSIGNALED(kid_status)));

		if ((maxkbytes > 0) && (max_vsize > maxkbytes))
		{
			kill(kid, SIGKILL);
		}

		if ((maxmillis > 0) && (info.utime_ms > maxmillis))
		{
			kill(kid, SIGKILL);
		}

	} while (!exit_flag);

	end = get_time();

#ifdef INCLUDE_EXIT_CODE
	if (WIFEXITED(kid_status))
	{
		fprintf(stderr, "Exit [%d]\n", WEXITSTATUS(kid_status));
	}
	else
	{
		fprintf(stderr, "Killed [%d]\n", WTERMSIG(kid_status));
	}
#endif

	{
		double kid_utime = ((double)kid_usage.ru_utime.tv_sec + (double)kid_usage.ru_utime.tv_usec / 1E6);
		double kid_stime = ((double)kid_usage.ru_stime.tv_sec + (double)kid_usage.ru_stime.tv_usec / 1E6);
		/* % cpu is (total cpu time)/(elapsed time).  */
		double elapsed_time_ms = (double)(end - start);

		double vv_ms = (kid_usage.ru_utime.tv_sec + kid_usage.ru_stime.tv_sec) * 1000 + (kid_usage.ru_utime.tv_usec + kid_usage.ru_stime.tv_usec) / 1000;

		if (elapsed_time_ms > 0)
		{
			unsigned cpu_time = (unsigned)(vv_ms * 100 / elapsed_time_ms);

			fprintf(stderr, "___OX_PERF_START___{\"cpu_usage\": %u,\"user_time\": %.2f, \"system_time\": %.2f , \"total_time\": %.2f,"
							"\"memory_peak\": %ld}___OX_PERF_END___\n",
					cpu_time, kid_utime, kid_stime, (double)(end - start) / 1000.0,
					max_vsize);
		}
	}

	exit(EXIT_SUCCESS);
}
