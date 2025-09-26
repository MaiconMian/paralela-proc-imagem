#ifndef UTILS_H_  
#define UTILS_H_

#include "image_utils.h"

typedef struct recebimentos {
    bool recebido;
    int bloco_atual;
    int numero_estrelas;
    int numero_estrelas_metade;
    int numero_estrelas_inteiras;
    int * numero_bordas_estrela;
    int * bordas;
} recebimentos;

int menor_divisor_k(int valor, int k);
int * cria_vetor_enviar(int*pixels, int linha_atual, int coluna_atual, int linha_max, int coluna_max, int numero_colunas);
void envia_vetor(int rank_destino, int bloco_atual, int * coluna_atual, int * linha_atual, image * img, int tamanho_linha_bloco, int tamanho_coluna_bloco);
int recebe_vetor (recebimentos * recebidos);
bool compara_vetores(int direcoes[8], int direcao[8]);
int direcao_estrela(int * direcoes);
bool direcao_estrela_recebida(int * direcoes, int direcao);
int retorna_estrela_util(recebimentos * recebidos, int bloco, int estrela_atual);
int verifica_paridade(recebimentos * recebidos, int bloco_atual, int bloco_pareo, int direcao, int bloco_coluna, int total_divisorias);
void resolve_estrelas_metade(int total_divisorias, recebimentos * recebidos, int bloco_coluna);

#endif