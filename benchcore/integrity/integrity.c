#include "integrity.h"
#include <malloc.h>

void insert_bug(int fd, int block_size, int offset){

  char bug_buffer[block_size+1];
  int i = 0;

  /* buffer full of 'b's */
  for(i=0; i < block_size;i++) 
    bug_buffer[i]='b';
  pwrite(fd, bug_buffer, block_size, offset);

}

/* Esta função é utilizada para popular o bufaux, ou seja, o bloco q vamos testar devia ser igual a este */
void get_block_content(generator_t *g, unsigned char **bufaux, block_info info_write, uint64_t block_size)
{
    if(info_write.flag_unique_block == 1){
        int res_gen = generate_data(g, bufaux, info_write.block_id, info_write.compression_index);
        if(res_gen == -1){
            printf("Fail in generate_data()");
            exit(0);
        }

        memcpy(&(*bufaux)[0], &info_write.cont_id, sizeof(unsigned int));
        /* Como o content_id mudou porque é aleatório no caso de blocos com 0 copias, vamos voltar a escrever o antigo */
        // for (size_t i = 0; i < sizeof(info_write.cont_id); ++i)
        //     *bufaux[i] = *((unsigned char *)&info_write.cont_id + i);
    }
    else{
        int res_gen = generate_data(g, bufaux, info_write.block_id, info_write.compression_index);
        if(res_gen == -1){
            printf("Fail in generate_data()");
            exit(0);
        }
    }
}

/* Esta função é chamada quando os 2 blocos são diferentes. buf é o buffer original que tem erros */
/* Esta função tenta "reparar" o buffer. Tenta identificar os ID's (3 ou 1) que caracterizam um bloco e construir um novo buffer a partir dessa informação */
int check_block_content(generator_t *g, unsigned char *buf, uint64_t block_size, block_info infowrite, duplicates_info *info)
{
    unsigned char *new_buffer;
    unsigned int cont_id = 0, block_id;

    memcpy(&cont_id, &buf[0], sizeof(unsigned int));
    memcpy(&block_id, &buf[sizeof(unsigned int)], sizeof(unsigned int));

    /* Significa que os bytes não correspondem a um ID de um bloco. */
    if (block_id >= info->zero_copy_blocks + info->duplicated_blocks) 
        return -1; /* Não conseguiu */

    block_info new_info;
    new_info.block_id = block_id;
    new_info.cont_id = cont_id;

    if(cont_id != block_id)
        new_info.flag_unique_block = 1;
    else
        new_info.flag_unique_block = 0;

    new_info.proc_id = -1;
    new_info.compression_index = get_block_compression_by_id(g, new_info.block_id);

    if(new_info.compression_index == -1){
        printf("Fail in get_block_compression_by_id()");
        exit(0);
    }
    else new_info.compression_index = new_info.compression_index / 10;

    get_block_content(g, &new_buffer, new_info, block_size);
    return memcmp(buf, new_buffer, block_size);  
}

/* Return: 1 se os bloco conter erros, 0 se ñ tem*/
/* Caso os blocos forem diferentes, tenta corrigir o bloco errado.*/
int online_check(generator_t *g, unsigned char *buf, block_info infowrite, uint64_t block_size, FILE *fpi, int final_check, duplicates_info *info)
{
    unsigned char *bufaux;
    int i = 0;
    get_block_content(g, &bufaux, infowrite, block_size); /* Inicializa o buffer auxiliar c/ o resultado esperado */

    /* Se o memcmp retornar 0 significa q os 2 blocos são iguais */
    /* Os blocos são diferentes */
    if (memcmp(buf, bufaux, block_size) != 0)
    {
        i = check_block_content(g, buf, block_size, infowrite, info);
        if (i == 0 && final_check == 0)
        {
            fprintf(fpi, "There was a mismatch regarding the last content written for the block is id %llu and the content read.\n", (long long unsigned int)infowrite.cont_id);
            fprintf(fpi, "Such is possible if the workload being ran is a mixed IO workload\n");
            fprintf(fpi, "Nevertheless the content of the block seems well built\n");
            return 1;
        }
        else
        {
            fprintf(fpi, "Error checking integrity for block with contentid %llu\n", (long long unsigned int)infowrite.cont_id);
            return 1;
        }
    }
    return 0;
}

/* Retorna o nr de blocos c/ erros */
/* Percorre todos os blocos testados e chama o compare_blocks */
int file_integrity(generator_t *g, int fd, struct user_confs *conf, struct duplicates_info *info, int idproc, FILE *fpi)
{

    unsigned char *buf;
    int res = 0;

    //memory block
    if (conf->odirectf == 1)
    {
        buf = memalign(conf->block_size, conf->block_size);
    }
    else
    {
        buf = malloc(conf->block_size);
    }

    //insert_bug(fd,100,100);

    uint64_t bytes_read = 0;
    while (bytes_read < conf->filesize)
    {
        /* Lê o próximo bloco */
        int res_pread = pread(fd, buf, conf->block_size, bytes_read); /* Read from fd to buf, nrbytes_to_read at a given offset (bytes_read). Retorna o nr de bytes lidos ou -1 (erro) */

        /* Erro ao ler o próximo bloco */
        if (res_pread <= 0)
        {
            printf("Reading block in offset %lu, with size %lu returned %d. Maybe the files was not populated correctly?\n", bytes_read, conf->block_size, res_pread);
            fprintf(fpi, "Reading block in offset %lu, with size %lu returned %d. Maybe the files was not populated correctly? The next integrity error is related with this message: ", bytes_read, conf->block_size, res_pread);
        }

        res += online_check(g, buf, info->content_tracker[idproc][bytes_read / conf->block_size], conf->block_size, fpi, 1, info);

        bytes_read += conf->block_size;
    }
    free(buf);

    return res;
}

/* Esta função cria um ficheiro c/ resultados do static integrity, abre fd para os ficheiros onde foram feitas escritas e 
faz o teste de integrity STATIC. No fim escreve para esses ficheiros os erros encontrados. */

int static_check(generator_t *g, struct user_confs *conf, struct duplicates_info *info)
{
    int i;
    int fd;
    int nprocs = 0;

    FILE *fpi = NULL;
    int integrity_errors = 0;
    char ifilename[PATH_SIZE];
    strcpy(ifilename, "./results/intgr_final_static_check");
    fpi = fopen(ifilename, "w");
    fprintf(fpi, "Final Integrity Check results\n");

    /* Calculo nrpocs */
    if (conf->mixedIO == 1)
    {
        nprocs = conf->nprocs / 2;
    }
    else
    {
        nprocs = conf->nprocs;
    }

    printf("File/device(s) integrity check is now Running...\n");

    /* filesystem */
    if (conf->rawdevice == 0)
    {

        //for each process populate its file with size filesize
        //we use DD for filling a non sparse image
        for (i = 0; i < nprocs; i++)
        {
            //create the file with unique name for process with id procid
            char name[PATH_SIZE];
            char id[4];
            sprintf(id, "%d", i);
            strcpy(name, conf->tempfilespath);
            strcat(name, TMP_FILE);
            strcat(name, id);

            printf("Running for proc %s...\n", name);

            fd = create_pfile(i, conf);
            integrity_errors += file_integrity(g, fd, conf, info, i, fpi);
            close(fd);
        }
    }
    /* rawdevice */
    else
    {
        fd = open_rawdev(conf->rawpath, conf);
        integrity_errors += file_integrity(g, fd, conf, info, 0, fpi);
        close(fd);
    }

    /* (f)printfs ... c/ resultados */
    if (integrity_errors > 0)
    {
        printf("Found %d integrity errors see %s file for more details\n", integrity_errors, ifilename);
        fprintf(fpi, "Found %d integrity errors.\n", integrity_errors);
    }
    else
    {
        fprintf(fpi, "No integrity issues found\n");
        printf("No integrity issues found\n");
    }

    fclose(fpi);
    printf("File/device(s) integrity check is completed\n");

    return integrity_errors;
}