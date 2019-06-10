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

typedef struct block_info{
	uint64_t cont_id;
	int procid;
	uint64_t ts;
	int flagUniqueBlock; /* 1 is unique, 0 shoud have copies */
} block_info;

typedef struct duplicates_info{

	//Number of distinct content blocks with duplicates
	uint64_t duplicated_blocks;

	//TOtal Number of blocks at the data set
	uint64_t total_blocks;

	uint64_t *statistics; /* Statistics it's an array wich each index represents an ID of a duplicated block and has as many entries as many block ID's duplicated */

	//shared mem
	uint64_t *zerodups;

	//Number blocks withouth duplicates
	//it refers to blocks
	//without any duplicate and not to unique blocks found at the storage
	uint64_t zero_copy_blocks;

	//unique counter for each process
  	//starts with value==max index at array sum
  	//since duplicated content is identified by number correspondent to the indexes at sum
  	//none will have a identifier bigger than this
  	uint64_t u_count;

  	struct block_info **content_tracker;
  
  	uint64_t topblock;
  	uint64_t botblock;
  	uint64_t topblock_dups;
  	uint64_t botblock_dups;
  	struct block_info last_unique_block;
  	struct block_info last_block_written;

} duplicates_info;

void get_distribution_stats(struct user_confs *conf, generator_t *g, duplicates_info *info, char* fname, int distout);
int gen_outputdist(duplicates_info *info, DB **dbpor,DB_ENV **envpor);
int compare_blocks(unsigned char* buf, block_info infowrite, uint64_t block_size, FILE* fpi, int finalcheck);
void get_block_content(unsigned char* bufaux, block_info infowrite, uint64_t block_size);
int next_block(duplicates_info *info, block_info *infowrite);

void get_writecontent2(unsigned char* buf, generator_t *g, int idproc, block_info *info_write);

#endif
