
/* Generator Wrapper to use in C */
#ifndef __GENERATORW_H__
#define __GENERATORW_H__

#include "../benchcore/duplicates/duplicatedist.h"

#ifdef __cplusplus
extern "C" {
#endif


generator_t *get_generator();
generator_t *get_generator2(unsigned int blockSize, unsigned int blocosToGenerate, unsigned int percentage_unique_blocks_analyze, 
    unsigned int compression_percentage_between_blocks, char* readPath);
void generator_destroy(generator_t *g);
int initialize(generator_t *g, struct duplicates_info *info);
void nextBlock(generator_t *g, unsigned char* buffer, struct block_info *info_write);

#ifdef __cplusplus
}
#endif

#endif /* __GENERATORW_H__ */
