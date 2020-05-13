#include <vector>
#include <tuple>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include "generator.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

unsigned int Generator::getRandomBlockFromLine(Linha l)
{
	uniform_int_distribution<unsigned int> dis(1, l.nrBlocks);
	// return dis(generator);
	return 1; // first block
}

/* Returns random line from inputfile */
Linha Generator::getLinha()
{
	return linhas.at(1);
	// return linhas.at(distribution_linhas(generator));
}

/* Returns compression index from a block */
unsigned int Generator::giveMyCompression(Linha linhaAleatoria, unsigned int randomBlockID)
{

	double myPercent = ((double)randomBlockID) / linhaAleatoria.nrBlocks * 100;
	double aux = 0;

	for (int i = 0; i < 10; i++)
	{
		aux += linhaAleatoria.compression.at(i);
		if (myPercent <= aux)
		{
			return i;
		}
	}
	return 9;
}

unsigned char *Generator::get_model_by_id_and_compression(unsigned int block_id, unsigned int compression)
{
	unsigned int indice = 0, somatorio_ids = this->atribuicao_blocos_unicos_para_modelos.at(indice);

	/* Diclo para calcular o indice do modelo */
	while (block_id > somatorio_ids && indice < this->atribuicao_blocos_unicos_para_modelos.size())
	{
		indice++;
		somatorio_ids += this->atribuicao_blocos_unicos_para_modelos.at(indice);
	}
	if (indice == this->atribuicao_blocos_unicos_para_modelos.size())
		return nullptr; /* ERRO !!!Nunca deve entrar nesta condição */
	else
		return modelos.at(indice).at(compression);
}

/* 
	Fill buffer with data given a blockID and compression.
	Se o blockID pertencer a um bloco com 0 copias é gerado um novo blockID (aleatorio)
	O buffer é preenchido artavés do modeo correspondente a esse block_id e posteriormente são alterado
	64*2 bits iniciais com: ID bloco e ID Conteudo. Quando ambos os ID's são iguais trata-se de um bloco com nr copias > 0.
*/
int Generator::generate_data(unsigned char **buffer, unsigned int block_id, unsigned int compression)
{
	unsigned int content_id = block_id;
	unsigned char *bufferModelo = get_model_by_id_and_compression(block_id, compression);

	/* Aleatoriadade na 1ª linha */
	for (Linha l : this->linhas)
	{
		if (l.nrCopies == 0)
		{
			if (block_id < l.nrBlocks)
			{
				uniform_int_distribution<unsigned int> dis(globalArgs.total_unique_blocks, generator.max());
				content_id = dis(generator);
			}
			break;
		}
	}
	/* Put (first) content id in buffer */
	memcpy(&bufferModelo[0], &content_id, sizeof(unsigned int));
	/* Put (second) block id in buffer */
	memcpy(&bufferModelo[sizeof(unsigned int)], &block_id, sizeof(unsigned int));

	*buffer = bufferModelo;

	return content_id;
}

void Generator::free_block_models()
{

	vector<vector<unsigned char *>>::iterator iterator_vectors;
	vector<unsigned char*>::iterator iterator_compressions;

	/* 100 vetores com 10 vetores cada (compressão) */
	for (iterator_vectors = modelos.begin(); iterator_vectors != modelos.end(); ++iterator_vectors)
	{
		for (iterator_compressions = (*iterator_vectors).begin(); iterator_compressions != (*iterator_vectors).end(); ++iterator_compressions)
		{
			free(*iterator_compressions);
		}
	}
}

/* Returns a block with a given size and compression. 
 * Compression must be between 1 and 99. 
 * The blockSize must be multiple of 4096
 */
unsigned char *Generator::blockModel(unsigned int blockSize, unsigned int compression, double seed, struct user_confs *conf)
{

	if (compression < 1 || compression > 99)
		return NULL;
	int size = blockSize / 4096;
	size = size * 4096; /* blockSize must be multiple of 4096 */
	int nrBlocos4096 = blockSize / 4096;

	//memory block
	unsigned char *buffer;
	int block_size = sizeof(unsigned char) * (size);
	if (conf->odirectf == 1)
	{
		buffer = (unsigned char *)memalign(block_size, block_size);
	}
	else
	{
		buffer = (unsigned char *)malloc(block_size);
	}

	/* Preencher o buffer com data random com 0% de compressão */
	for (int i = 0; i < nrBlocos4096; i++)
	{
		double c = 1;

		/* Gerar o bloco */
		for (int contador = 0; contador < 4096 / sizeof(c); contador++)
		{
			for (size_t i = 0; i < sizeof(c); ++i)
				buffer[(sizeof(c) * contador) + i] = *((unsigned char *)&c + i);
			c = c * seed;
		}
	}

	/* This code transforms a block with no compression to compress 'rate'%
    * rate is the rate compression (1 - 100) */
	if (buffer != NULL)
	{

		if (compression < 1 || compression > 100 || size <= 0 || buffer == NULL)
			return NULL;
		compression = compression + 0;
		/* Tamanho do semi-bloco a ser repetido (1% do tamanho total) */
		int nrBlocosADuplicar = size / 100, posicaoBuffer = 0, contador = 0;

		/* Começa na posicao 0 do buffer, vai andar de 1 em 1 % do buffer. 
        * Vai copiar os primeiros 1% dos bytes até à taxa pretendida. Ex: se a taxa for 40% vai copiar
        os primeiros 1% 40 vezes
        */

		while (contador < compression && posicaoBuffer < (size - nrBlocosADuplicar))
		{
			for (int posicaoAuxiliar = 0; posicaoAuxiliar < nrBlocosADuplicar; posicaoAuxiliar++)
				buffer[posicaoBuffer + posicaoAuxiliar] = buffer[posicaoAuxiliar];

			posicaoBuffer = posicaoBuffer + nrBlocosADuplicar;
			contador++;
		}
	}
	return buffer;
}

/* 
 * nr_model_vectors: nr of vectors loaded
 * return: 1 -> ok, -1 -> error 
 */
int Generator::loadModels(struct user_confs *conf)
{
	/* Gerar todos os vetores de modelos para memória */
	for (unsigned int vectores_created = 0; vectores_created < nr_models; vectores_created++)
	{
		/* Gerar um valor random (double) que vai ser utilizado para gerar um bloco sem compressão, 
        através da multiplicação sucessivada deste valor */
		/* Neste caso, como geramos apenas uma seed, todos os modelos vão ser iguais na parte que não comprime */

		uniform_real_distribution<double> dis(1.1, 1.9);
		double seed = dis(generator);

		vector<unsigned char *> new_vector;

		/* Gerar vetor de 10 modelos */
		for (int compression = 5; compression <= 95; compression = compression + 10)
		{
			unsigned char *buffer = blockModel(globalArgs.blockSize, compression, seed, conf);

			if (buffer != NULL)
				new_vector.push_back(buffer); /* Adicionar modelo ao vector */
			else
			{
				std::cout << "Error loading models..." << endl;
				return -1;
			}
		}
		modelos.push_back(new_vector); /* Adicionar vector de modelos ao vector com os vectores */
	}

	/* Atribuir os modelos a blocos */
	unsigned int bu = globalArgs.total_unique_blocks;
	double p = (double)globalArgs.percentagem_compressao_entre_blocos / (double)100;
	int perc_trying_to_achive = p * (get<1>(this->compressions_inter_blocks_interval) - get<0>(this->compressions_inter_blocks_interval)) + get<0>(this->compressions_inter_blocks_interval);
	std::cout << "Trying to achive " << perc_trying_to_achive << " compression between 2 random blocks." << endl;
	unsigned int x = sqrt(p) * bu, r = bu - x, y = r / (nr_models - 1), z = r % (nr_models - 1), w = (nr_models - 1) - z;
	for (unsigned int i = 0; i < 1; i++)
		this->atribuicao_blocos_unicos_para_modelos.push_back(x);
	for (unsigned int i = 0; i < w; i++)
		this->atribuicao_blocos_unicos_para_modelos.push_back(y);
	for (unsigned int i = 0; i < z; i++)
		this->atribuicao_blocos_unicos_para_modelos.push_back(y + 1);

	return 1;
}

/* return block compression or -1 (error) */
int Generator::get_block_compression_by_id(int block_id)
{

	Linha l;
	int min_id_line, max_id_line, id_line;

	for (vector<Linha>::iterator it = linhas.begin(); it != linhas.end(); ++it)
	{
		min_id_line = it->nrBase + 1;
		max_id_line = it->nrBase + it->nrBlocks;
		id_line = block_id - it->nrBase;

		if (block_id >= min_id_line && block_id <= max_id_line)
		{ /* É esta a linha do bloco */
			int x = giveMyCompression(*it, id_line);
			return x * 10;
		}
	}
	return -1;
}

/* return tuple (min_media, max_media) */
tuple<double, double> Generator::get_media_inter(int percentage_interval)
{
	/* Generate new block */
	uniform_int_distribution<> dis(1, globalArgs.total_unique_blocks);

	int id_block_analyze = 1, previous_compression = -1, current_compression = -1, nr_pairs = 0;
	int new_media_max = 0, new_media_min = 0, total_media_min = 0, total_media_max = 0;
	int minimum = 0, maximum = 0;

	if (!(percentage_interval > 0 && percentage_interval < 100))
		return make_tuple(-1, -1);

	for (int i = 0; i < globalArgs.total_unique_blocks * percentage_interval / 100; i++)
	{
		
		id_block_analyze = dis(generator);

		/* Get compression */
		current_compression = get_block_compression_by_id(id_block_analyze);
		//std::cout << "Block: " << id_block_analyze << "\tCompress: " << current_compression << endl;

		/* Calculate media */
		if (previous_compression == -1)
		{
			previous_compression = current_compression; /* É o 1º elemento do par para calcular a media */
		}
		else
		{
			new_media_max = (current_compression + previous_compression + min((100 - current_compression), (100 - previous_compression))) / 2;
			new_media_min = (current_compression + previous_compression) / 2;
			nr_pairs++;
			previous_compression = -1;
			minimum += new_media_min;
			maximum += new_media_max;
		}
	}
	std::cout << "Compression interval estimated based on " << nr_pairs << " pairs of block."
			  << "\t" << endl;
	std::cout << "Estimated Compression Interval: [" << minimum / nr_pairs << ", " << maximum / nr_pairs << "]" << endl;

	return make_tuple(minimum / nr_pairs, maximum / nr_pairs);
}

Generator::Generator(unsigned int blockSize, unsigned int blocosToGenerate,
					 unsigned int percentage_unique_blocks_analyze, unsigned int compression_percentage_between_blocks, string readPath)
{
	globalArgs.blockSize = blockSize;
	globalArgs.blocosAGerar = blocosToGenerate;
	globalArgs.total_unique_blocks = 0;
	globalArgs.percentage_unique_blocks_analyze = percentage_unique_blocks_analyze;
	globalArgs.percentagem_compressao_entre_blocos = compression_percentage_between_blocks;
	globalArgs.readPath = readPath;
	nr_models = 100;
}

void Generator:: nextBlock(unsigned char **buffer, block_info *info_write)
{
	Linha linhaAleatoria = getLinha();

	unsigned int random_block_id_line = getRandomBlockFromLine(linhaAleatoria);

	int compression_index = giveMyCompression(linhaAleatoria, random_block_id_line);

	unsigned int block_id = random_block_id_line + linhaAleatoria.nrBase;
	info_write->block_id = block_id;

	block_id = generate_data(buffer, block_id, compression_index);

	info_write->cont_id = block_id;
	info_write->compression_index = compression_index;
	linhaAleatoria.nrCopies == 0 ? info_write->flag_unique_block = 1 : info_write->flag_unique_block = 0; /* Flag block has 0 copies */
}

void Generator::initialize_random(){
	random_device rd;
	generator = std::mt19937(rd());
}

/* Read input file for duplicate and compression distribution.
    Generate Models to generate data
 * path: path to input file
 * block_size: size of each block
 * nrBlocksToGenerate: number of blocks generated
 * percentage: percetage of unique blocks used to estimate compression inter blocks
 * struct info is needed to fill some variables
 * Return 1:ok -1:error
*/
int Generator::initialize(duplicates_info *info, struct user_confs *conf)
{
	initialize_random();

	if (globalArgs.blockSize < 4096)
		return -1;
	if (!(globalArgs.percentagem_compressao_entre_blocos >= 1 && globalArgs.percentagem_compressao_entre_blocos <= 100))
		return -1; /* Variavel tem q estar no intervalo [1,100] */

	globalArgs.zeroCopiesBlocks = 0;
	globalArgs.total_unique_blocks = 0;

	/* Reading Input File */
	unsigned int nrBaseAux = 0;
	string line;

	std::ifstream fin(globalArgs.readPath);
	if (!fin)
	{
		std::cout << "Can't open input file!" << globalArgs.readPath << endl;
		return -1;
	}

	while (getline(fin, line))
	{
		Linha newLine;

		newLine.nrBase = nrBaseAux;
		std::stringstream ss(line);
		string token;
		double probabilidadeLinha;

		if (getline(ss, token, ' '))
		{
			newLine.nrCopies = stoi(token);

			if (getline(ss, token, ' '))
			{
				probabilidadeLinha = stof(token);

				newLine.nrBlocks = (unsigned int)(globalArgs.blocosAGerar * (probabilidadeLinha / 100) / (newLine.nrCopies + 1));
				if (newLine.nrBlocks == 0)
					newLine.nrBlocks = 1;
				nrBaseAux = nrBaseAux + newLine.nrBlocks;
			}

			if (newLine.nrCopies == 0)
			{
				globalArgs.zeroCopiesBlocks = newLine.nrBlocks; /* Number of unique blocks that doesn't have copies */
			}
			globalArgs.total_unique_blocks += newLine.nrBlocks; /* Sum of all unique blocks */
		}

		/* restantes eltos da linha (compressoes) */
		while (getline(ss, token, ' '))
		{
			newLine.compression.push_back(stof(token));
		}

		this->linhas.push_back(newLine);
		this->weights.push_back(probabilidadeLinha);
	}

	cout << endl << "----------Deduplication----------" << endl;
	cout << "Total blocks: " << globalArgs.blocosAGerar << " (" << (globalArgs.blocosAGerar * globalArgs.blockSize / 1e6) << " MB)" << endl ;
	cout << "Blocks with no duplicates: " << globalArgs.zeroCopiesBlocks << endl;
	cout << "Different blocks with duplicates: " << globalArgs.total_unique_blocks - globalArgs.zeroCopiesBlocks << endl;
	cout << "-----------------------------------------" << endl << endl;
 
	/* Initialize discrete_distribution to generate random lines based on their weights (used in getLinha function) */
	distribution_linhas = discrete_distribution<int>(weights.begin(), weights.end());

	/* Calcular o intervalo esperado de compressão entre blocos*/
	cout  << "----------Compression----------" << endl;
	compressions_inter_blocks_interval = get_media_inter(globalArgs.percentage_unique_blocks_analyze);

	/* Generating Block Models */
	int returnLoadModels = loadModels(conf);
	cout << "--------------------------------" << endl << endl;


	/* Fill struct duplicates_info */
	info->zero_copy_blocks = globalArgs.zeroCopiesBlocks;
	info->duplicated_blocks = globalArgs.total_unique_blocks - globalArgs.zeroCopiesBlocks;
	info->total_blocks = globalArgs.blocosAGerar;
	info->u_count = info->duplicated_blocks + 1;

	return returnLoadModels;
}