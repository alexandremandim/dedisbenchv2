/* DEDISbench
 * (c) 2010 2017 INESC TEC and U. Minho
 * Written by J. Paulo
 */

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "duplicatedist.h"
#include "../../utils/random/random.h"
#include "../../libGenerator/generatorW.h"

/* Esta função:
    Inicializa a biblioteca libGenerator 'g'; Preenche alguns campos de 'info'
    Inicializa o statistics a 0's
*/
void init(generator_t *g, duplicates_info *info, int distout)
{

	int i = initialize(g, info);
	if (i != 1)
	{	
		printf("Error: initialize generator.\n");
		exit(0);
	}
	else
	{
		printf("Initialize generator library with success.\n");
	}

	/* Initialize info->statistics to zero */
	int z;
	for (z = 1; z < info->duplicated_blocks; z++)
		if (distout == 1)
			info->statistics[z] = 0;
}

void get_writecontent(unsigned char **buf, generator_t *g, uint64_t idproc, block_info *info_write)
{
	/* Preenche buffer & infos about block */
	nextBlock(g, buf, info_write);
	info_write->proc_id = idproc;
}

/* Utilizado para fazer logging do que realmente foi escrito */
int gen_outputdist(duplicates_info *info, DB **dbpor, DB_ENV **envpor)
{

	FILE *f = fopen("headerdist", "wb");
	if (!f)
	{
		perror("fopen");
	}

	uint64_t i = 0;
	for (i = 0; i < info->duplicated_blocks; i++)
	{
		//The array has the number of occurrences of a specific block
		//the blocks
		if (info->statistics[i] == 1)
		{
			*info->zerodups = *info->zerodups + 1;

			uint64_t block_id = i + info->zero_copy_blocks; /* ID's start with 0 copies blocks */
			fprintf(f, "%lu %lu\n", block_id, info->statistics[i]);
		}
		if (info->statistics[i] > 1)
		{

			struct hash_value hvalue;
			uint64_t ndups = info->statistics[i] - 1;

			//see if entry already exists and
			//Insert new value in hash for print number_of_duplicates->numberof blocks
			int retprint = get_db_print(&ndups, &hvalue, dbpor, envpor);

			//if hash entry does not exist
			if (retprint == DB_NOTFOUND)
			{

				hvalue.cont = 1;
				//insert into into the hashtable
				put_db_print(&ndups, &hvalue, dbpor, envpor);
			}
			else
			{

				//increase counter
				hvalue.cont++;
				//insert counter in the right entry
				put_db_print(&ndups, &hvalue, dbpor, envpor);
			}
			uint64_t block_id = i + info->zero_copy_blocks; /* ID's start with 0 copies blocks */
			fprintf(f, "%lu %lu\n", block_id, info->statistics[i]);
		}
	}
	//insert zeroduplicates now
	struct hash_value hvalue;
	uint64_t ndups = 0;

	//see if entry already exists and
	//Insert new value in hash for print number_of_duplicates->numberof blocks
	int retprint = get_db_print(&ndups, &hvalue, dbpor, envpor);

	//if hash entry does not exist
	if (retprint == DB_NOTFOUND)
	{

		hvalue.cont = *info->zerodups;
		//insert into into the hashtable
		put_db_print(&ndups, &hvalue, dbpor, envpor);
	}
	else
	{
		//increase counter
		hvalue.cont += *info->zerodups;
		//insert counter in the right entry
		put_db_print(&ndups, &hvalue, dbpor, envpor);
	}

	fclose(f);
	return 0;
}