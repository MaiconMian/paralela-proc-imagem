/*=============================================================
* UNIFAL = Universidade Federal de Alfenas.
* BACHARELADO EM CIENCIA DA COMPUTACAO.
* Trabalho.. : Contagem de estrelas em paralelo
* Professor. : Paulo Alexandre Bressan
* Aluno..... : Maicon Almeida Mian - 2023.1.08.013
* Data...... : 24/09/2025
*=============================================================*/

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "utils.h"
#include "image_utils.h"

#define DEU_CERTO 0
#define E_BORDA_IMAGEM 1
#define DEU_ERRADO 2
#define TOCA_BORDA_MAS_NAO_CONTINUA 3

#define ESQUERDA 5
#define CIMA 6
#define DIAGONAL_DIREITA 7
#define DIAGONAL_DIAGONAL 8
#define DIAGONAL_BAIXO 9

#define DIREITA 10
#define BAIXO 11
#define DIAGONAL 12
#define OUTRO_ANGULO 13
#define VETOR_VAZIO 14

/**
 * Função responsável por retornar o menor divisor maior ou igual que k.
 * @param   valor  Valor inicial.
 * @param   k      Limiar inferior do divisor.
 * @return         Retorna o divisor.
 */
int menor_divisor_k (int valor, int k){

    for (int i = 1; i <= valor; i++){
        if(valor % i == 0 && i >= k){
            return i;
        }
    }

    return 1;
}

/**
 * Função responsável por criar um vetor que será enviado a cada processo.
 * 
 * @param pixels       Ponteiro para o vetor 1D que contém todos os pixels da imagem.
 * @param linha_atual  Índice da linha inicial do bloco a ser copiado.
 * @param coluna_atual Índice da coluna inicial do bloco a ser copiado.
 * @param linha_max    Número de linhas que o bloco terá.
 * @param coluna_max   Número de colunas que o bloco terá.
 * @param numero_colunas Número total de colunas da imagem original.
 *
 * @return Retorna um ponteiro para um vetor recém-alocado contendo os pixels do bloco.
 */
int * cria_vetor_enviar(int*pixels, int linha_atual, int coluna_atual, int linha_max, int coluna_max, int numero_colunas){

    int *vetor = malloc(sizeof(int) * linha_max * coluna_max);

    for (int i = 0; i < linha_max; i++){
        for(int j = 0; j < coluna_max; j++){
            vetor[i * coluna_max + j] = pixels[ (i+linha_atual) * numero_colunas  + (j+coluna_atual) ];
        }
    }
    return vetor;
}

/**
 * Envia um bloco da imagem para um processo específico via MPI.

 * @param rank_destino          Rank do processo que vai receber o bloco.
 * @param bloco_atual           Número do bloco atual.
 * @param coluna_atual          Ponteiro para a coluna atual.
 * @param linha_atual           Ponteiro para a linha atual.
 * @param img                   Ponteiro para a estrutura da imagem.
 * @param tamanho_linha_bloco   Número de linhas do bloco.
 * @param tamanho_coluna_bloco  Número de colunas do bloco.
 */
void envia_vetor(int rank_destino, int bloco_atual, int * coluna_atual, int * linha_atual, image * img, int tamanho_linha_bloco, int tamanho_coluna_bloco){

    int * vetor_a_enviar = cria_vetor_enviar((*img).pixels, *linha_atual, *coluna_atual, tamanho_linha_bloco, tamanho_coluna_bloco, (*img).numero_colunas);
    
    int informacoes[4];
    informacoes[0] = bloco_atual;
    informacoes[1] = tamanho_linha_bloco;
    informacoes[2] = tamanho_coluna_bloco;
    informacoes[3] = (*img).numero_colunas;

    MPI_Send(informacoes, 4, MPI_INT, rank_destino, 0, MPI_COMM_WORLD);
    MPI_Send(vetor_a_enviar, tamanho_linha_bloco * tamanho_coluna_bloco, MPI_INT, rank_destino, 1, MPI_COMM_WORLD);

    *coluna_atual += tamanho_coluna_bloco;
    if (*coluna_atual >= (*img).numero_colunas) {
        *coluna_atual = 0;
        *linha_atual += tamanho_linha_bloco;
    }

    free(vetor_a_enviar);

}

/**
 * Recebe um bloco de informações de outro processo via MPI.
 * @param recebidos Ponteiro para o array de estruturas de recebimentos.
 *
 * @return          Rank do processo que enviou o bloco.
 */
int recebe_vetor (recebimentos * recebidos) {

    int bloco_atual;
    int numero_estrelas;
    int numero_estrelas_metade;

    MPI_Status status;

    MPI_Recv(&bloco_atual, 1, MPI_INT, MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, &status);
    MPI_Recv(&numero_estrelas, 1, MPI_INT, status.MPI_SOURCE, 5, MPI_COMM_WORLD, &status);
    MPI_Recv(&numero_estrelas_metade, 1, MPI_INT, status.MPI_SOURCE, 6, MPI_COMM_WORLD, &status);

    int *numero_bordas_estrela = malloc(numero_estrelas * sizeof(int));
    MPI_Recv(numero_bordas_estrela, numero_estrelas, MPI_INT, status.MPI_SOURCE, 7, MPI_COMM_WORLD, &status);
        
    int *bordas = malloc(numero_estrelas_metade * 8 * sizeof(int));
    MPI_Recv(bordas, numero_estrelas_metade * 8, MPI_INT, status.MPI_SOURCE, 8, MPI_COMM_WORLD, &status);
        
    recebidos[bloco_atual].numero_bordas_estrela = numero_bordas_estrela;
    recebidos[bloco_atual].bordas = bordas;
    recebidos[bloco_atual].recebido = true;
    recebidos[bloco_atual].bloco_atual = bloco_atual;
    recebidos[bloco_atual].numero_estrelas = numero_estrelas;
    recebidos[bloco_atual].numero_estrelas_inteiras = numero_estrelas - numero_estrelas_metade;
    recebidos[bloco_atual].numero_estrelas_metade = numero_estrelas_metade;

    return status.MPI_SOURCE;

}

/**
 * Compara dois vatores de direções e retorna verdadeiro se forem iguais

 * @param direcoes  Primeiro vetor de direções.
 * @param direcao   Segundo vetor de direções.
 *
 * @return          rue se os forem iguais, false caso contrário.
 */
bool compara_vetores(int direcoes[8], int direcao[8]){
    for(int i = 0; i < 8; i++){
        if(direcoes[i] != direcao[i]){
            return false;
        }
    }
    return true;
}

/**
 * Determina a direção que uma estrela continua com base no vetor de direções.
 * @param direcoes  Vetor de 8 posições representando direções.
 *
 * @return          Código da direção da estrela.
 */
int direcao_estrela(int * direcoes){

    int direita[] = {0,0,0,0,1,0,0,0};
    int baixo[] = {0,0,0,0,0,0,1,0};
    int diagonal[] = {0,0,0,0,1,0,1,1};

    if(direcoes[0] == -1){
        return VETOR_VAZIO;
    } else if (compara_vetores(direcoes, direita)){
        return DIREITA;
    } else if (compara_vetores(direcoes, baixo)){
        return BAIXO;
    } else if (compara_vetores(direcoes, diagonal)){
        return DIAGONAL;
    }
    return OUTRO_ANGULO;
}

/**
 * Verifica se a estrela continua na direção necessária.
 * @param direcoes Vetor de 8 posições representando direções.
 * @param direcao  Código da direção a verificar.

 * @return         true se o vetor corresponde à direção, false caso contrário.
 */
bool direcao_estrela_recebida(int * direcoes, int direcao){

    int esquerda[] =           {0,0,0,1,0,0,0,0};
    int cima[] =               {0,1,0,0,0,0,0,0};
    int diagonal_direita[] =   {0,0,0,1,0,1,1,0};
    int diagonal_diagonal[] =  {1,1,0,1,0,0,0,0};
    int diagonal_baixo[] =     {0,1,1,0,1,0,0,0};

    if(direcao == ESQUERDA){
        return compara_vetores(direcoes, esquerda);
    } else if (direcao == CIMA){
        return compara_vetores(direcoes, cima);
    } else if (direcao == DIAGONAL_DIREITA){
        return compara_vetores(direcoes, diagonal_direita);
    } else if (direcao == DIAGONAL_DIAGONAL){
        return compara_vetores(direcoes, diagonal_diagonal);
    } else if (direcao == DIAGONAL_BAIXO) {
        return compara_vetores(direcoes, diagonal_baixo);
    }

    return false;

}

/**
 * Retorna o índice da próxima estrela válida em um bloco.
 * @param recebidos   Array de estruturas de recebimentos.
 * @param bloco       Índice do bloco.
 * @param estrela_atual Índice da estrela a partir da qual buscar.

 * @return            Índice da próxima estrela válida, ou -1 se não houver.
 */
int retorna_estrela_util(recebimentos * recebidos, int bloco, int estrela_atual){

    if(estrela_atual == recebidos[bloco].numero_estrelas_metade) return -1;

    for(int j = estrela_atual; j < recebidos[bloco].numero_estrelas_metade; j++){
        if(recebidos[bloco].bordas[ j * 8 ] != -1){
            break;
        }
        estrela_atual++;
    }
    return estrela_atual;

}

/**
 * Verifica se se algum vetor de direções de estrela é correspondente ao necessário para o bloco atual.
 * @param recebidos       Array de estruturas de recebimentos.
 * @param bloco_atual     Índice do bloco atual.
 * @param bloco_pareo     Índice do bloco vizinho.
 * @param direcao         Direção da estrela a ser verificada.
 * @param bloco_coluna    Número de blocos por linha.
 * @param total_divisorias Número total de blocos.
 *
 * @return                Código indicando o resultado.
 */
int verifica_paridade(recebimentos * recebidos, int bloco_atual, int bloco_pareo, int direcao, int bloco_coluna, int total_divisorias){

    int linha = bloco_atual / bloco_coluna;
    int coluna = bloco_atual % bloco_coluna;
    int n_linhas = total_divisorias / bloco_coluna;

    if (direcao == DIREITA && coluna == bloco_coluna - 1) {
        return E_BORDA_IMAGEM;
    }
    if (direcao == BAIXO && linha == n_linhas - 1) {
        return E_BORDA_IMAGEM;
    }
    if (direcao == DIAGONAL && (coluna == bloco_coluna - 1 || linha == n_linhas - 1)) {
        return E_BORDA_IMAGEM;
    }
    if (bloco_pareo < 0 || bloco_pareo >= total_divisorias) {
        return E_BORDA_IMAGEM;
    }

    if(!recebidos[bloco_pareo].recebido) {
        return TOCA_BORDA_MAS_NAO_CONTINUA;
    }

    if(recebidos[bloco_pareo].numero_estrelas_metade == 0){
        return TOCA_BORDA_MAS_NAO_CONTINUA;
    }

    int estrela = retorna_estrela_util(recebidos,bloco_pareo,0);

    while (estrela != -1){
        int vetor_direcoes_atual[8];
        for(int k = 0; k < 8; k++){
            vetor_direcoes_atual[k] = recebidos[bloco_pareo].bordas[ estrela * 8 + k];
        }

        if(direcao_estrela_recebida(vetor_direcoes_atual, direcao)){
            recebidos[bloco_pareo].numero_estrelas_metade--;
            recebidos[bloco_pareo].bordas[estrela*8] = -1;
            return DEU_CERTO;
        } 
        estrela = retorna_estrela_util(recebidos,bloco_pareo,estrela+1);
    }

    return TOCA_BORDA_MAS_NAO_CONTINUA;
}

/**
 * Resolve todas as estrelas que atravessam processos.
 *
 * @param total_divisorias  Número total de blocos.
 * @param recebidos         Estruturas de recebimentos.
 * @param bloco_coluna      Número de blocos por linha.
 */
void resolve_estrelas_metade(int total_divisorias, recebimentos * recebidos, int bloco_coluna){

    int desloc[] = {1, bloco_coluna, bloco_coluna+1};

    for (int i = 0; i < total_divisorias; i++) {
        while (recebidos[i].numero_estrelas_metade > 0){
            int estrela_atual = retorna_estrela_util(recebidos, i, 0);
            if (estrela_atual == -1) {
                break; 
            }
            int vetor_direcoes_atual[8];
            for(int k = 0; k < 8; k++){
                vetor_direcoes_atual[k] = recebidos[i].bordas[ estrela_atual * 8 + k];
            }

            int dir = direcao_estrela(vetor_direcoes_atual), resultado, resultado_aux_1, resultado_aux_2;

            switch (dir) {
                case DIREITA:
                    resultado = verifica_paridade(recebidos, i, i+desloc[0], ESQUERDA, bloco_coluna, total_divisorias);
                    break; 
                case BAIXO:
                    resultado = verifica_paridade(recebidos, i, i+desloc[1], CIMA, bloco_coluna, total_divisorias);
                    break;
                case DIAGONAL:
                    resultado = verifica_paridade(recebidos, i, i+desloc[0], DIAGONAL_DIREITA, bloco_coluna, total_divisorias);
                    resultado_aux_1 = verifica_paridade(recebidos, i, i+desloc[1], DIAGONAL_BAIXO, bloco_coluna, total_divisorias);
                    resultado_aux_2 = verifica_paridade(recebidos, i, i+desloc[2], DIAGONAL_DIAGONAL, bloco_coluna, total_divisorias);
                    if (resultado == DEU_ERRADO || resultado_aux_1 == DEU_ERRADO || resultado_aux_2 == DEU_ERRADO) {
                        resultado = DEU_ERRADO;
                    }
                    break;
                case OUTRO_ANGULO:
                    resultado = DEU_CERTO;
                    break;
                default: 
                    break;
            }

            if(resultado != DEU_ERRADO){
                recebidos[i].numero_estrelas_metade--;
                recebidos[i].numero_estrelas_inteiras++;
                recebidos[i].bordas[estrela_atual*8] = -1;
            } else {
                break;
            }
        }
    }
}


