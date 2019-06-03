#include "generator.h"
#include "generatorW.h"
#include "../benchcore/duplicates/duplicatedist.h"

struct generator{
    void* obj;
};

generator_t *get_generator(){
    generator_t* r;
    Generator* g;

    r = (typeof(r))malloc(sizeof(*r));
    g = new Generator();
    r->obj = g;
    return r;
}

generator_t *get_generator2(unsigned int blockSize, unsigned int blocosToGenerate, unsigned int percentage_unique_blocks_analyze, 
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
    delete static_cast<Generator *>(g->obj);
	free(g);
}

int initialize(generator_t *g, struct duplicates_info *info){
    if(g == NULL || g->obj == NULL) return -1;
    
    Generator *gen = static_cast<Generator *>(g->obj);
    return gen->initialize(struct duplicates_info *info);
}

void nextBlock(generator_t *g, unsigned char* buffer, struct block_info *info_write){
    if(g == NULL || g->obj == NULL) return;
    
    Generator *gen = static_cast<Generator *>(g->obj);
    gen->nextBlock(buffer, info_write);
}