/* DEDISbench
 * (c) 2010 2017 INESC TEC and U. Minho
 * Written by J. Paulo
 */

#include "io.h"
#include "../utils/random/random.h"

int init_io(struct user_confs *conf, int procid)
{

	//init random generator
	//if the seed is always the same the generator generates the same numbers
	//for each proces the seed = seed + processid or all the processes would
	//generate the same load
	init_rand(conf->seed + procid);

	init_ioposition(conf);

	return 0;
}

uint64_t write_request(generator_t *g, unsigned char **buf, int idproc, struct block_info *infowrite, struct user_confs *conf, struct stats *stat)
{
	get_writecontent(buf, g, idproc, infowrite);
	return get_ioposition(conf, stat, idproc);
}

uint64_t read_request(struct user_confs *conf, struct stats *stat, int idproc)
{
	return get_ioposition(conf, stat, idproc);
}
