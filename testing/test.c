#include "test.h"


/* Retorna tempo de resposta */
/* Este metodo vai realiar um milhao de iteracoes e vai devolver o tempo_resposta*/
double single_test(generator_t *g, struct user_confs *conf, int nrChamadas){

	clock_t tempo_total;
	unsigned char *buf;				
	struct block_info info_write;
	struct stats stat = {.beginio = -1};

	/* TEMPO DE RESPOSTA E DEBITO */

    tempo_total = clock(); 
	for(int i = 0; i < nrChamadas; i++)
	{
		write_request(g, &buf, 0, &info_write, conf, &stat);
	}	
		
	tempo_total = clock() - tempo_total;
    
	double tempo_de_resposta = ((double)tempo_total)/CLOCKS_PER_SEC;
	return tempo_de_resposta;
}

/* Debito, Tempo de resposta, Media e DP p operacao */
void launch_test_speed(generator_t *g, struct user_confs *conf)
{

	printf("Started test speed\n\n");
	
    int nrChamadas = 1000000;
	double nrIteracoes = 10;
	double debitosOperacao[100];
	double somatorioDebitos = 0;

	/* Ciclo principal */
	for(int i = 0; i < nrIteracoes; i++){
		debitosOperacao[i] = ((double)nrChamadas)/(single_test(g, conf, nrChamadas));
		printf("Débito: %.2f Op / s.\n", debitosOperacao[i]);
		somatorioDebitos += debitosOperacao[i];
	}

    double mediaDebito = somatorioDebitos/nrIteracoes;
	printf("Débito: %.2f Op / s.\n", mediaDebito);

	double somatorioDesvioPadrao = 0;
	for(int z = 0; z < nrIteracoes; z++){
		somatorioDesvioPadrao += pow((debitosOperacao[z] - mediaDebito),2);
	}

	double desvioPadraoDebito = sqrt(somatorioDesvioPadrao/nrIteracoes);

	printf("Desvio Padrão: %.6f.\n", desvioPadraoDebito);
}