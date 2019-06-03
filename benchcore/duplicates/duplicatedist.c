/* DEDISbench
 * (c) 2010 2017 INESC TEC and U. Minho
 * Written by J. Paulo
 */


#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "duplicatedist.h"
#include "../../utils/random/random.h"
#include "../../libGenerator/generatorW.h"


/* Esta função percorre o input e preenche: 
    duplicates_info->zero_copy_blocks c/ o number_blocks correspondentes; 
    duplicates_info->duplicated_blocks c/ o nr total de ID's de blocos duplicados (somatório do nb de linhas em que o nc é > 0)
    duplicates_info->total_blocks c/ o nr de blocos totais (nc+1)*nb
    duplicates_info->u_count c/ o duplicates_info->duplicated_blocks + 1

    Tb inicializa o statistics
*/
void get_distribution_stats(struct user_confs *conf, generator_t *g, struct duplicates_info *info, char* fname, int distout){

	g = get_generator2(conf->block_size, conf->input_total_blocks, conf->percentage_analyze , conf->compression_to_achieve, fname);
	int i = initialize(g, info);
	if(i != 1){printf("Error: initialize generator.\n"); exit(0);}

  /* Initialize info->statistics to zero */
  int z;
  for(z=1;z<info->duplicated_blocks;z++)
    if(distout==1)
    	info->statistics[z]=0;

}

/* Utilizado para a integridade*/
void get_block_content(char* bufaux, struct block_info infowrite, uint64_t block_size){

  //initialize the buffer with duplicate content
  int bufp = 0;
  for(bufp=0;bufp<block_size;bufp++){
    bufaux[bufp] = 'a';
  }

  if(infowrite.procid!=-1){
    sprintf(bufaux,"%llu pid %d time %llu ", (long long unsigned int)infowrite.cont_id,infowrite.procid,(long long unsigned int)infowrite.ts);
  }
  else{
    sprintf(bufaux,"%llu ", (long long unsigned int)infowrite.cont_id);
  }

}

/* Utilizado para a integridade */
int check_block_content(char* buf, uint64_t block_size){

  const char s[2] = " ";
  char *token=NULL;
  int contwrites_b=-1;
  int times_b=-1;
  uint64_t contwrites=0;
  uint64_t times=0;
  int pids=-1;

  char original_buf[block_size];
  memcpy(original_buf,buf, block_size);

  //initialize the buffer with duplicate content
  int bufp = 0;
  char bufaux[block_size];
  for(bufp=0;bufp<block_size;bufp++){
    bufaux[bufp] = 'a';
  }
   
  token = strtok(original_buf, s);
  /* walk through other tokens */
  while( token != NULL ) 
  {

      if(contwrites_b<0){
        contwrites = atoll(token);
        contwrites_b=1;
      }

      if(pids<0 && strcmp(token,"pid")==0){
        token = strtok(NULL, s);
        pids=atoi(token);
        
      }else{
        if(times_b<0 && strcmp(token,"time")==0){
          token = strtok(NULL, s);
          times = atoll(token);
          times_b=1;
        }
        else{
          token = strtok(NULL, s);
        }
      }
      
  }

  if(contwrites_b >= 0 && pids>=0 && times_b >=0){
    sprintf(bufaux,"%llu pid %d time %llu ", (long long unsigned int)contwrites,pids,(long long unsigned int)times);
  }
  else{
    if(contwrites_b>=0){
      sprintf(bufaux,"%llu ", (long long unsigned int)contwrites);
    }
    else{
      return -1;
    }
  }
    
  return memcmp(buf,bufaux, block_size);
   
}

/* Utiizado para a integridade */
int compare_blocks(char* buf, struct block_info infowrite, uint64_t block_size, FILE* fpi, int final_check){

  unsigned char bufaux[block_size];
  int i=0;
  get_block_content(bufaux, infowrite, block_size);

  if(memcmp(buf,bufaux,block_size)!=0){
    i=check_block_content(buf, block_size);
    if(i==0 && final_check==0){
      fprintf(fpi,"There was a mismatch regarding the last content written for the block is id %llu and the content read.\n", (long long unsigned int) infowrite.cont_id);
      fprintf(fpi,"Such is possible if the workload being ran is a mixed IO workload\n");
      fprintf(fpi,"Nevertheless the content of the block seems well built\n");
      return 1;
    }else{
      fprintf(fpi, "Error checking integrity for block with contentid %llu\n", (long long unsigned int) infowrite.cont_id);
      return 1;
    }
  }
  return 0;
}

void get_writecontent2(char* buf, generator_t *g, int idproc, struct block_info *info_write){

  nextBlock(g, buf, info_write);
  
  if(info_write->flagUniqueBlock == 1){
    struct timeval tim;
    gettimeofday(&tim, NULL);
    uint64_t tunique=tim.tv_sec*1000000+(tim.tv_usec);
    info_write->ts = tunique;
    info_write->procid = idproc;
  }
  else{
    info_write->procid=-1;
    info_write->ts=-1;
  }

}

/* Utilizado para fazer logging do que realmente foi escrito */
int gen_outputdist(struct duplicates_info *info, DB **dbpor,DB_ENV **envpor){

	FILE* f = fopen("headerdist", "wb");
	if(!f) {
		perror("fopen");
	}

	uint64_t i=0;
	for(i=0;i<info->duplicated_blocks;i++){
		//The array has the number of occurrences of a specific block
		//the blocks
		if(info->statistics[i]==1){
			*info->zerodups=*info->zerodups+1;

      int block_id = i + info->zero_copy_blocks; /* ID's start with 0 copies blocks */
			fprintf(f, "%lu %lu\n", block_id, info->statistics[i]);
		}
		if(info->statistics[i]>1){

			struct hash_value hvalue;
			uint64_t ndups = info->statistics[i]-1;

			 //see if entry already exists and
			 //Insert new value in hash for print number_of_duplicates->numberof blocks
			 int retprint = get_db_print(&ndups,&hvalue,dbpor,envpor);

			 //if hash entry does not exist
			 if(retprint == DB_NOTFOUND){

				  hvalue.cont=1;
				  //insert into into the hashtable
				  put_db_print(&ndups,&hvalue,dbpor,envpor);
			 }
			 else{

				  //increase counter
				  hvalue.cont++;
				  //insert counter in the right entry
				  put_db_print(&ndups,&hvalue,dbpor,envpor);
			 }
      int block_id = i + info->zero_copy_blocks; /* ID's start with 0 copies blocks */
			fprintf(f, "%lu %lu\n", i, info->statistics[i]);
		}

	}
	//insert zeroduplicates now
	struct hash_value hvalue;
	uint64_t ndups = 0;

	//see if entry already exists and
	//Insert new value in hash for print number_of_duplicates->numberof blocks
	int retprint = get_db_print(&ndups,&hvalue,dbpor,envpor);

	//if hash entry does not exist
	if(retprint == DB_NOTFOUND){

		hvalue.cont=*info->zerodups;
		//insert into into the hashtable
		put_db_print(&ndups,&hvalue,dbpor,envpor);
	}
	else{
	  //increase counter
	  hvalue.cont+=*info->zerodups;
	  //insert counter in the right entry
	  put_db_print(&ndups,&hvalue,dbpor,envpor);
    }

	fclose(f);
	return 0;
}
