#include "test.h"

/* Debito, Tempo de resposta, Media e DP p operacao */
void launch_test_speed(generator_t *g, struct user_confs *conf, int nr_ciclos){

	double tempo_de_resposta = 0; /* Tempo que demora a executar 1 000 000 operações */
	int debito = 0; /* Nr operações por segundo */
	double media_por_operacao = 0; /* media de tempo que cada operação demora */
	double desvio_padrao_operacao = 0; /* Desvio padrão p/ operaação */
	double temposOperacao[nr_ciclos];	

	unsigned char *buf;				
	struct block_info info_write;
	struct stats stat = {.beginio = -1};

	printf("Started test speed\n\n");

	clock_t tempo_total;
	clock_t tempo_por_operacao;

	/* TEMPO DE RESPOSTA E DEBITO */

    tempo_total = clock(); 
	for(int i = 0; i < nr_ciclos; i++)	write_request(g, &buf, 0, &info_write, conf, &stat);
	tempo_total = clock() - tempo_total;
    
	tempo_de_resposta = ((double)tempo_total)/CLOCKS_PER_SEC;
	debito = (double)nr_ciclos/tempo_de_resposta;

	/* MEDIA E DESVIO PADRAO P/ OPERACAO */
	for(int i = 0; i < nr_ciclos; i++){
		tempo_por_operacao = clock();  
		write_request(g, &buf, 0, &info_write, conf, &stat);
		tempo_por_operacao = clock() - tempo_por_operacao;
		temposOperacao[i] = ((double)tempo_por_operacao)/CLOCKS_PER_SEC;
	}

	/* Ciclo para calcular a media */
	double aux_media = 0;
	for(int i = 0; i < nr_ciclos; i++){
		aux_media += temposOperacao[i];
	}
	media_por_operacao =  aux_media/nr_ciclos; /* Media em segundos */

	/* Calculo do desvio padrão */
	double aux_desvio_padrao = 0;
	for(int i = 0; i < nr_ciclos; i++){
		aux_desvio_padrao += pow((temposOperacao[i] - media_por_operacao),2);
	}
	desvio_padrao_operacao = sqrt((aux_desvio_padrao/nr_ciclos));

	printf("Tempo de resposta (%d operações): %f segundos.\n", nr_ciclos, tempo_de_resposta);
	printf("Débito: %d operações/segundo.\n", debito);
	printf("Media por operação: %.10f us.\n", (media_por_operacao*(double)1000000));
	printf("Desvio padrão por operação: %.10f us.\n\n", (desvio_padrao_operacao*(double)1000000));
}