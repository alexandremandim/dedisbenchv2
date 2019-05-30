/* Generator Wrapper to use in C */
#ifndef __GENERATORW_H__
#define __GENERATORW_H__

#ifdef __cplusplus
extern "C" {
#endif

struct generator;
typedef struct generator generator_t;

generator_t *get_generator();
generator_t *get_generator2(unsigned int blockSize, unsigned int blocosToGenerate, unsigned int percentage_unique_blocks_analyze, 
    unsigned int compression_percentage_between_blocks, char* readPath);
void generator_destroy(generator_t *g);
int initialize(generator_t *g);
void nextBlock(generator_t *g, unsigned char* buffer);

#ifdef __cplusplus
}
#endif

#endif /* __GENERATORW_H__ */
