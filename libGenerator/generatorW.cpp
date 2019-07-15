#include "generator.h"
#include "generatorW.h"
#include "../benchcore/duplicates/duplicatedist.h"
struct generator{
    void* obj;
};

generator_t *get_generator(unsigned int blockSize, unsigned int blocosToGenerate, unsigned int percentage_unique_blocks_analyze, 
                            unsigned int compression_percentage_between_blocks, char* readPath){
    generator_t* r;
    Generator* g;

    r = (typeof(r))malloc(sizeof(*r));
    g = new Generator(blockSize, blocosToGenerate, percentage_unique_blocks_analyze, compression_percentage_between_blocks, string(readPath));
    r->obj = g;
    return r;
}

void generator_destroy(generator_t *g){
    if(g == NULL || g->obj == NULL) return;

    Generator *gen = static_cast<Generator *>(g->obj);
    gen->free_block_models();
    delete static_cast<Generator *>(g->obj);
	free(g);
}

int initialize(generator_t *g, duplicates_info *info, struct user_confs *conf){
    if(g == NULL || g->obj == NULL) return -1;
    
    Generator *gen = static_cast<Generator *>(g->obj);
    return gen->initialize(info, conf);
}

void nextBlock(generator_t *g, unsigned char** buffer, block_info *info_write){
    if(g == NULL || g->obj == NULL) return;
    
    Generator *gen = static_cast<Generator *>(g->obj);
    gen->nextBlock(buffer, info_write);
}

int generate_data(generator_t *g, unsigned char** buffer, unsigned int block_id, unsigned int compression){
    if(g == NULL || g->obj == NULL) return -1;

    Generator *gen = static_cast<Generator *>(g->obj);
    return gen->generate_data(buffer, block_id, compression);
}

int get_block_compression_by_id(generator_t *g, int block_id){
    if(g == NULL || g->obj == NULL) return -1;

    Generator *gen = static_cast<Generator *>(g->obj);
    return gen->get_block_compression_by_id(block_id);
}

void initialize_random(generator_t *g){
   if(g == NULL || g->obj == NULL) return;
    
    Generator *gen = static_cast<Generator *>(g->obj);
    gen->initialize_random(); 
}