/* DEDISbench
 * (c) 2010 2017 INESC TEC and U. Minho
 * Written by J. Paulo
 */

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <sys/types.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/random/random.h"
#include "populate.h"
#include "../benchcore/io.h"

int open_rawdev(char *rawpath, struct user_confs *conf)
{

	int fd_test;
	if (conf->odirectf == 1)
	{
		fd_test = open(rawpath, O_RDWR | O_LARGEFILE | O_DIRECT, 0644);
		if (fd_test == -1)
		{
			perror("Error opening file for process I/O O_DIRECT");
			exit(0);
		}
	}
	else
	{
		fd_test = open(rawpath, O_RDWR | O_LARGEFILE, 0644);
		if (fd_test == -1)
		{
			perror("Error opening file for process I/O");
			exit(0);
		}
	}
	return fd_test;
}

//we must check this
//create the file where the process will perform I/O operations
int create_pfile(int procid, struct user_confs *conf)
{

	int fd_test;

	//create the file with unique name for process with id procid
	char name[PATH_SIZE];
	char id[4];
	sprintf(id, "%d", procid);
	strcpy(name, conf->tempfilespath);
	strcat(name, TMP_FILE);
	strcat(name, id);

	//printf("%s\n", conf->tempfilespath);
	if (conf->odirectf == 1)
	{
		//printf("opening %s with O_DIRECT\n",name);
		//device where the process will write
		fd_test = open(name, O_RDWR | O_LARGEFILE | O_CREAT | O_DIRECT, 0644);
	}
	else
	{
		//printf("opening %s\n",name);
		//device where the process will write
		fd_test = open(name, O_RDWR | O_LARGEFILE | O_CREAT, 0644);
	}
	if (fd_test == -1)
	{
		perror("Error opening file for process I/O");
		exit(0);
	}

	return fd_test;
}

int destroy_pfile(int procid, struct user_confs *conf)
{

	//create the file with unique name for process with id procid
	char name[PATH_SIZE];
	char id[4];
	sprintf(id, "%d", procid);
	strcpy(name, "rm ");
	strcat(name, conf->tempfilespath);
	strcat(name, TMP_FILE);
	strcat(name, id);

	printf("performing %s\n", name);

	int ret = system(name);
	if (ret < 0)
	{
		perror("System rm failed");
	}

	return 0;
}

uint64_t dd_populate(char *name, struct user_confs *conf)
{

	//create the file with unique name for process with id procid
	char command[PATH_SIZE];
	char count[10];
	sprintf(count, "%llu", (long long unsigned int)conf->filesize / 1024 / 1024);
	strcpy(command, "dd if=/dev/zero of=");
	strcat(command, name);
	strcat(command, " bs=1M count=");
	strcat(command, count);

	printf("populating file/device %s with %s\n", name, command);
	int ret = system(command);
	if (ret < 0)
	{
		perror("System dd failed");
	}

	return conf->filesize;
}

uint64_t real_populate(generator_t *g, int fd, struct user_confs *conf, struct duplicates_info *info, int idproc)
{
	struct stats stat;

	//init random generator
	//if the seed is always the same the generator generates the same numbers
	//for each proces the seed = seed + processid or all the processes would
	//here is seed+nrprocesses so that in the population the load is different
	//generate the same load
	init_rand(conf->seed + conf->nprocs);

	uint64_t bytes_written = 0;
	while (bytes_written < conf->filesize)
	{

		unsigned char *buf;
		struct block_info info_write;

		//write_request(g, &buf, idproc, &info_write, conf, &stat);
		get_writecontent(&buf, g, idproc, &info_write);

		if (conf->distout == 1)
		{
			uint64_t idwrite = info_write.cont_id;
			uint64_t indexID = idwrite - info->zero_copy_blocks;

			if (info_write.flag_unique_block == 0)
			{ /* It's a ID with >=1 copies */
				info->statistics[indexID]++;
				if (info->statistics[indexID] <= 1)
				{
					stat.uni++;
				}
			}
		}

		int res = pwrite(fd, buf, conf->block_size, bytes_written);
		if (res < conf->block_size)
		{
			perror("Error populating file");
		}

		if (conf->integrity >= 1)
		{
			int pos = (conf->rawdevice == 1) ? 0 : idproc;

			info->content_tracker[pos][bytes_written / conf->block_size].cont_id = info_write.cont_id;
			info->content_tracker[pos][bytes_written / conf->block_size].proc_id = info_write.proc_id;
			info->content_tracker[pos][bytes_written / conf->block_size].block_id = info_write.block_id;
			info->content_tracker[pos][bytes_written / conf->block_size].compression_index = info_write.compression_index;
			info->content_tracker[pos][bytes_written / conf->block_size].flag_unique_block = info_write.flag_unique_block;
		}
		bytes_written += conf->block_size;
	}
	return bytes_written;
}

//populate files with content
void populate(generator_t *g, struct user_confs *conf, struct duplicates_info *info)
{
	/* Initial time */
    uint64_t prev_time_value, time_value;
	uint64_t time_diff;
	prev_time_value = get_posix_clock_time();

	printf("----------Populating----------\n");
	int i;
	int fd;
	int nprocs = 0;

	if (conf->mixedIO == 1)
	{
		// If running mixed test with only one proc this will zero
		// and it wont start the populate
		nprocs = conf->nprocs / 2;
	}
	else
	{
		nprocs = conf->nprocs;
	}

	uint64_t bytes_populated = 0;

	if (conf->rawdevice == 0)
	{

		//for each process populate its file with size filesize
		//we use DD for filling a non sparse image
		//
		//(!conf->mixedIO && i<nprocs) || (conf->mixedIO && i<nprocs)
		for (i = 0; i < nprocs; i++)
		{
			//create the file with unique name for process with id procid
			char name[PATH_SIZE];
			char id[4];
			sprintf(id, "%d", i);
			strcpy(name, conf->tempfilespath);
			strcat(name, TMP_FILE);
			strcat(name, id);

			if (conf->populate == DDPOP)
			{
				bytes_populated += dd_populate(name, conf);
			}
			else
			{
				printf("Populating file %s with realistic content\n", name);
				fd = create_pfile(i, conf);
				bytes_populated += real_populate(g, fd, conf, info, i);
				fsync(fd);
				close(fd);
			}
		}
	}
	else
	{

		if (conf->populate == DDPOP)
		{
			bytes_populated += dd_populate(conf->rawpath, conf);
		}
		else
		{

			printf("Populating device %s with realistic content\n", conf->rawpath);

			fd = open_rawdev(conf->rawpath, conf);
			bytes_populated += real_populate(g, fd, conf, info, 0);
			fsync(fd);
			close(fd);
		}
	}
    /* Final time */
	time_value = get_posix_clock_time();

	/* Time difference */
	time_diff = time_value - prev_time_value;
    double throughput = (double)bytes_populated/(time_diff/1e6)/1e6;
    printf("Took %.2f sec (%.2f min) to populate.\nThroughput %.2f MB/s \n", time_diff/1e6, (time_diff/1e6)/60,throughput);

	printf("File/device(s) population is completed wrote %llu bytes (%.2f MB)\n", (unsigned long long int)bytes_populated, bytes_populated/1e6);
	printf("------------------------------\n");
}