#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <tuple>
#include <string>
#include <random>
#include "../benchcore/duplicates/duplicatedist.h"
#include "../structs/defines.h"

using namespace std;

class Linha{
    public:
        unsigned int nrCopies, nrBlocks, nrBase;
        std::vector<double> compression; 
};

class Generator {
    public:
        Generator(unsigned int blockSize, unsigned int blocosToGenerate, unsigned int percentage_unique_blocks_analyze, unsigned int compression_percentage_between_blocks, string readPath);
        void nextBlock(unsigned char** buffer, block_info *info_write);
        int initialize(duplicates_info *info, struct user_confs *conf);
        int generate_data(unsigned char** buffer, unsigned int block_id, unsigned int compression);
        int get_block_compression_by_id(int block_id);        
        void free_block_models();
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
        int nr_models;
        /* RANDOM */
        std::mt19937 generator;
        discrete_distribution<int> distribution_linhas;


        unsigned int getRandomBlockFromLine(Linha l);
        Linha getLinha();
        unsigned int giveMyCompression(Linha linhaAleatoria, unsigned int randomBlockID);
        unsigned char* blockModel(unsigned int blockSize, unsigned int compression, double seed, struct user_confs *conf);
        int loadModels(struct user_confs *conf);
        tuple<double, double> get_media_inter(int percentage_interval);
        unsigned char* get_model_by_id_and_compression(unsigned int block_id, unsigned int compression);
      
};
#endif /* GENERATOR_H */