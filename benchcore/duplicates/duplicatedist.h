/* DEDISbench
 * (c) 2010 2017 INESC TEC and U. Minho
 * Written by J. Paulo
 */

#ifndef DUPLICATEDIST_H
#define DUPLICATEDIST_H

#include <stdint.h>
#include "../../utils/db/berk.h"
#include "../../structs/defines.h"
//#include "../../libGenerator/generatorW.h"

#define HEADF "results/headerdist"

struct generator;
typedef struct generator generator_t;

typedef struct block_info
{
	unsigned int cont_id;
	uint64_t proc_id;
	unsigned int block_id;
	unsigned int compression_index;
	int flag_unique_block; /* 1 is unique, 0 shoud have copies */
} block_info;

typedef struct duplicates_info
{

	/* Nr de blocos totais que vão ser "gerados" */
	uint64_t total_blocks;
	
	/* Nr de blocos com 0 copias */
	uint64_t zero_copy_blocks;
	/* Nr de blocos com > 0 copias */
	uint64_t duplicated_blocks;

	uint64_t *statistics; /* Statistics it's an array wich each index represents an ID of a duplicated block and has as many entries as many block ID's duplicated */

	//shared mem
	uint64_t *zerodups;

	//Number blocks withouth duplicates
	//it refers to blocks
	//without any duplicate and not to unique blocks found at the storage
	

	//unique counter for each process
	//starts with value==max index at array sum
	//since duplicated content is identified by number correspondent to the indexes at sum
	//none will have a identifier bigger than this
	uint64_t u_count;

	struct block_info **content_tracker;

} duplicates_info;

void init(generator_t *g, duplicates_info *info, int distout);
int gen_outputdist(duplicates_info *info, DB **dbpor, DB_ENV **envpor);
void get_writecontent(unsigned char **buf, generator_t *g, uint64_t idproc, block_info *info_write);

#endif
