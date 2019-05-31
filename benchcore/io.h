/* DEDISbench
 * (c) 2010 2017 INESC TEC and U. Minho
 * Written by J. Paulo
 */

#ifndef IO_H
#define IO_H


#include "accesses/iodist.h"
#include "duplicates/duplicatedist.h"
#include "../structs/defines.h"

#include "../libGenerator/generatorW.h"

int init_io(struct user_confs *conf, int procid);
uint64_t write_request(generator_t *g, char* buf, struct user_confs *conf, struct duplicates_info *info, struct stats *stat, int idproc, struct block_info *infowrite);
uint64_t read_request(struct user_confs *conf, struct stats *stat, int idproc);
uint64_t write_request2(generator_t *g, char* buf, int idproc, struct block_info *infowrite, struct user_confs *conf, struct stats *stat);

#endif