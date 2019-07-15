#include "test.h"


/* Retorna tempo de resposta */
double single_test(generator_t *g, struct user_confs *conf, int nr_ciclos){

	clock_t tempo_total;
	clock_t tempo_por_operacao;
	unsigned char *buf;				
	struct block_info info_write;
	struct stats stat = {.beginio = -1};

	/* TEMPO DE RESPOSTA E DEBITO */

    tempo_total = clock(); 
	for(int i = 0; i < nr_ciclos; i++)	write_request(g, &buf, 0, &info_write, conf, &stat);
	tempo_total = clock() - tempo_total;
    
	double tempo_de_resposta = ((double)tempo_total)/CLOCKS_PER_SEC;
	return tempo_de_resposta;
}

/* Debito, Tempo de resposta, Media e DP p operacao */
void launch_test_speed(generator_t *g, struct user_confs *conf, int nr_ciclos){

	printf("Started test speed\n\n");

	/* Ciclo principal */
	int x = 10, i;
	double temposOperacao[x];	
	double debitosOperacao[x];
	for(i = 0; i < x; i++){
		temposOperacao[i] = single_test(g, conf, nr_ciclos);
		debitosOperacao[i] = (double)nr_ciclos/temposOperacao[i];
	}

	/* Ciclo para calcular a media */
	double aux_media_tr = 0, aux_media_deb = 0;
	for(i = 0; i < x; i++){
		aux_media_tr += temposOperacao[i];
		aux_media_deb += debitosOperacao[i];
	}
	double tr_medio =  aux_media_tr/10; /* Media em segundos */
	double debito_medio = aux_media_deb/10;	/* Op / seg */
	printf("Débito médio: %.2f Op / s.\n", debito_medio);
	printf("TR médio:  %.6f s / %d operações.\n", tr_medio, nr_ciclos);
	printf("TR médio p/ operação: %.6f us.\n", (1/debito_medio)*(double)1000000);

	/* Calculo do desvio padrão */
	double aux_desvio_padrao = 0;
	for(int i = 0; i < x; i++){
		aux_desvio_padrao += pow((temposOperacao[i] - tr_medio),2);
	}
	double desvio_padrao_operacao = sqrt((aux_desvio_padrao/x));

	printf("Desvio Padrão TR médio: %.6f.\n" , desvio_padrao_operacao);
}