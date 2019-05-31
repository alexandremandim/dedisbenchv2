/* DEDISbench
 * (c) 2010 2017 INESC TEC and U. Minho
 * Written by J. Paulo
 */

#ifndef DUPLICATEDIST_H
#define DUPLICATEDIST_H

#include <stdint.h>
#include "../../utils/db/berk.h"
#include "../../structs/defines.h"

#include "../../libGenerator/generatorW.h"

#define HEADF "results/headerdist"

struct block_info{
	uint64_t cont_id;
	int procid;
	uint64_t ts;
	int flagUniqueBlock; /* 1 is unique, 0 shoud have copies */
};

struct duplicates_info{

	//Number of distinct content blocks with duplicates
	uint64_t duplicated_blocks;

	//TOtal Number of blocks at the data set
	uint64_t total_blocks;

	uint64_t *statistics;

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

};

void get_distribution_stats(struct user_confs *conf, generator_t *g, struct duplicates_info *info, char* fname, int distout);
int gen_outputdist(struct duplicates_info *info, DB **dbpor,DB_ENV **envpor);
int compare_blocks(char* buf, struct block_info infowrite, uint64_t block_size, FILE* fpi, int finalcheck);
void get_block_content(char* bufaux, struct block_info infowrite, uint64_t block_size);
int next_block(struct duplicates_info *info, struct block_info *infowrite);

void get_writecontent2(char* buf, generator_t *g, int idproc, struct block_info *info_write);

#endif
