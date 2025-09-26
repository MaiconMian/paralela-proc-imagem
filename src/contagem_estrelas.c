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
#include "contagem_estrelas.h"

#define SUPERIOR_ESQUERDO 0
#define CIMA    1
#define SUPERIOR_DIREITO 2
#define ESQ     3
#define DIR     4
#define INFERIOR_ESQUERDO 5
#define BAIXO   6
#define INFERIOR_DIREITO 7

/**
 * Verifica se os indices i j estão dentro dos limites da matriz.
 *
 * @param i       Linha a ser verificada.
 * @param j       Coluna a ser verificada.
 * @param linhas  Número total de linhas da matriz.
 * @param colunas Número total de colunas da matriz.
 *
 * @return        true se a posição estiver dentro da matriz, false caso contrário.
 */
bool dentro(int i, int j, int linhas, int colunas) {
    return (i >= 0 && i < linhas && j >= 0 && j < colunas);
}

/**
 * Inicializa uma estrutura estrela com os valores padrão.
 * @param estrelas    Ponteiro para as estrelas.
 * @param atual_index Ponteiro para o índice atual (negativo) da estrela a ser inicializada.
 */
void inicia_estrela(estrela * estrelas, int * atual_index){
    estrelas[ (*atual_index*(-1)) - 1 ].indice = *atual_index;
    estrelas[ (*atual_index*(-1)) - 1 ].e_borda = false;
    for(int i = 0; i < 8; i++){
        estrelas[ (*atual_index*(-1)) - 1 ].vetor_borda[i] = 0;
    }
    (*atual_index)--;
}

/**
 * Retorna o ID de uma estrela vizinha de uma posição específica na matriz.
 *
 * @param i         Linha da célula atual.
 * @param j         Coluna da célula atual.
 * @param linhas    Número total de linhas da matriz.
 * @param colunas   Número total de colunas da matriz.
 * @param matriz    Matriz que contém IDs das estrelas (negativos).
 * @param desloc    Array de deslocamentos para os 8 vizinhos.
 *
 * @return          ID da estrela vizinha encontrada, ou 0 se nenhuma.
 */
int pega_id_vizinho(int i, int j, int linhas, int colunas, int matriz[linhas][colunas], int desloc[8][2]) {
    for (int k = 0; k < 8; k++) {
        int nova_linha = i + desloc[k][0];
        int nova_coluna = j + desloc[k][1];
        if (dentro(nova_linha, nova_coluna, linhas, colunas) && matriz[nova_linha][nova_coluna] < 0) {
            return matriz[nova_linha][nova_coluna]; 
        }
    }
    return 0; 
}

/**
 * Marca o vetor de borda de uma estrela mostrando qual a direção provavel que ela continua.
 *
 * @param estrelas Ponteiro para as estrelas.
 * @param index    Índice da estrela a ser atualizada.
 * @param i        Linha da célula atual.
 * @param j        Coluna da célula atual.
 * @param linhas   Número total de linhas da matriz.
 * @param colunas  Número total de colunas da matriz.
 */
void marca_vetor_borda(estrela *estrelas, int index, int i, int j, int linhas, int colunas){
    
    if (i == 0 || i == linhas-1 || j == 0 || j == colunas - 1) estrelas[index].e_borda = true;
    if (i == 0) estrelas[index].vetor_borda[CIMA] = 1;              
    if (i == linhas-1) estrelas[index].vetor_borda[BAIXO] = 1;      
    if (j == 0) estrelas[index].vetor_borda[ESQ] = 1;               
    if (j == colunas-1) estrelas[index].vetor_borda[DIR] = 1;   
    if (i == 0 && j == 0) estrelas[index].vetor_borda[SUPERIOR_ESQUERDO] = 1;
    if (i == 0 && j == colunas-1) estrelas[index].vetor_borda[SUPERIOR_DIREITO] = 1;
    if (i == linhas-1 && j == 0) estrelas[index].vetor_borda[INFERIOR_ESQUERDO] = 1;
    if (i == linhas-1 && j == colunas-1) estrelas[index].vetor_borda[INFERIOR_DIREITO] = 1;

}

/**
 * Conta as estrelas em uma parte da imagem, identificando estrelas inteiras e bordas.
 *
 * @param quantidade_estrelas               Ponteiro para variável que armazenará a quantidade total de estrelas encontradas.
 * @param quantidades_estrelas_inteiras     Ponteiro para variável que armazenará o número de estrelas inteiras (não encostam na borda).
 * @param limiar                            Valor do limiar para considerar um pixel como parte de uma estrela.
 * @param informacoes                       Vetor com informações sobre o bloco da imagem.
 * @param numero_linhas                     Número de linhas da matriz da parte da imagem.
 * @param numero_colunas                    Número de colunas da matriz da parte da imagem.
 * @param parte_imagem                      Matriz contendo a parte da imagem a ser analisada.
 *
 * @return                                  Ponteiro para o array de estruturas estrela.
 */
estrela * conta_estrelas(int * quantidade_estrelas, int * quantidades_estrelas_inteiras, int limiar, int * informacoes, int numero_linhas, int numero_colunas, int parte_imagem[numero_linhas][numero_colunas]){

    estrela *estrelas = malloc(sizeof(estrela) * informacoes[1] * informacoes[2]);
    if(estrelas == NULL) {
        printf("ERRO: malloc falhou\n");
        return NULL;
    }
    int atual_index = -1; 
    int desloc[8][2] = {
        {-1, -1}, 
        {-1,  0}, 
        {-1,  1}, 
        { 0,  1}, 
        { 1,  1}, 
        { 1,  0}, 
        { 1, -1}, 
        { 0, -1}  
    };

    int id;
    for(int i = 0; i < numero_linhas; i++){
        for(int j = 0; j < numero_colunas; j++){
            if(parte_imagem[i][j] >= limiar){
                id = pega_id_vizinho(i, j, numero_linhas, numero_colunas, parte_imagem, desloc);
                if(id < 0){
                    parte_imagem[i][j] = id;
                } else if (id == 0){
                    id = atual_index;
                    parte_imagem[i][j] = id;
                    inicia_estrela(estrelas, &atual_index);
                    (*quantidade_estrelas)++;
                }
                marca_vetor_borda(estrelas, (id*-1)-1, i, j, numero_linhas, numero_colunas);
            }
        }
    }

    for(int i = 0; i < *quantidade_estrelas; i++){
        if(!estrelas[i].e_borda) {
            (*quantidades_estrelas_inteiras)++;
        }
    }


    return estrelas;
}
