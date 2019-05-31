#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <tuple>
#include <string>
#include "../benchcore/duplicates/duplicatedist.h"

using namespace std;

class Linha{
    public:
        unsigned int nrCopies, nrBlocks, nrBase;
        std::vector<double> compression; 
};

class Generator {
    public:

        Generator();
        Generator(unsigned int blockSize, unsigned int blocosToGenerate, unsigned int percentage_unique_blocks_analyze, unsigned int compression_percentage_between_blocks, string readPath);
        void nextBlock(unsigned char* buffer, struct block_info *info_write);
        /* Read input file for duplicate and cokmpression distribution.
            Generate Models to generate data
            * path: path to input file
            * block_size: size of each block
            * nrBlocksToGenerate: number of blocks generated
            * percentage: percetage of unique blocks used to estimate compression inter blocks
            * Return 1:ok -1:error
        */
        int initialize(struct duplicates_info *info);

    private:
    
        struct globalArgs_t {
            unsigned int blockSize, blocosAGerar, percentage_unique_blocks_analyze, percentagem_compressao_entre_blocos;
            unsigned int zeroCopiesBlocks, total_unique_blocks /*unique blocks with copies too*/;
            string readPath;
        } globalArgs;
        tuple<double, double> compressions_inter_blocks_interval;
        vector<Linha> linhas;
        vector<double> weights;
        vector<vector<unsigned char*>> modelos;     /* Vector com 100 modelos  Cada modelo tem 10 blocos (compressao 0, 10, 20 , ..., 100) */
        vector<unsigned int> atribuicao_blocos_unicos_para_modelos;

        unsigned int getRandomBlockFromLine(Linha l);
        Linha getLinha();
        unsigned int giveMyCompression(Linha linhaAleatoria, unsigned int randomBlockID);
        void generate_data(unsigned char* buffer, unsigned int blockKey, unsigned int compression);
        unsigned char* blockModel(unsigned int blockSize, unsigned int compression, double seed);
        int loadModels();
        int get_block_compression_by_id(int block_id);
        tuple<double, double> get_media_inter(int percentage_interval);
      
};
#endif /* GENERATOR_H */