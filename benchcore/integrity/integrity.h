#ifndef INTEGRITY_H
#define INTEGRITY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../populate/populate.h"
#include "../../structs/defines.h"
#include "../duplicates/duplicatedist.h"
#include "../../libGenerator/generatorW.h"

/* 
    Check if a block integrity is correct.
    Returns 0 : correct or 1 : incorrect 
*/
int online_check(generator_t *g, unsigned char *buf, block_info infowrite, uint64_t block_size, FILE *fpi, int final_check, duplicates_info *info);
/* 
    Check if all blocks are correct. Try to repair incorrect ones.
    Returns nr of incorrect blocks.
*/
int static_check(generator_t *g, struct user_confs *conf, struct duplicates_info *info);
#endif