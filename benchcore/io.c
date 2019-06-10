/* DEDISbench
 * (c) 2010 2017 INESC TEC and U. Minho
 * Written by J. Paulo
 */

#include "io.h"
#include "../utils/random/random.h"


int init_io(struct user_confs *conf, int procid){

  //init random generator
  //if the seed is always the same the generator generates the same numbers
  //for each proces the seed = seed + processid or all the processes would
  //generate the same load
  init_rand(conf->seed+procid);

  init_ioposition(conf);

  return 0;
}

uint64_t write_request2(generator_t *g, unsigned char* buf, int idproc, struct block_info *infowrite, struct user_confs *conf, struct stats *stat){

  /*Atenção: Nesta versão ignorei o facto de inserir no buffer um timestamp e o id do processo quando ele é único,
    uma vez que a geração aleatória de ID's deixa de ser feita dentro de um intervalo e passa a ser um nr aleatório qualquer
    eliminando assim QUASE por completo a hipótese de vários processos/benchmarks escolherem os mesmos ID's.
  */
 
  get_writecontent2(buf, g, idproc, infowrite);

  return get_ioposition(conf, stat, idproc);
}

uint64_t read_request(struct user_confs *conf, struct stats *stat, int idproc){

  return get_ioposition(conf, stat, idproc);

}
