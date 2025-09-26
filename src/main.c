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
#include "image_utils.h"
#include "utils.h"
#include "contagem_estrelas.h"

int main(int argc, char *argv[]){

    MPI_Init(NULL, NULL);

    int numero_processos;
    MPI_Comm_size(MPI_COMM_WORLD, &numero_processos);
    int processo_atual;
    MPI_Comm_rank(MPI_COMM_WORLD, &processo_atual);

    if (argc < 2) {

        if (processo_atual == 0) {
            printf("\tO uso do programa é:\n");
            printf("\tmpirun -np [numero_processos] --hostfile hostfile.txt contar-estrelas [nome_arquivo].pgm\n");
        }

        MPI_Finalize();  
        return 0;
    }

    if(processo_atual == 0){

        char *nome_arquivo = argv[1];
        int maior_diametro = 32;
        image *img = read_image(nome_arquivo);

        printf("\n\t Lendo a imagem %s\n", nome_arquivo);
        
        int tamanho_linha_bloco = menor_divisor_k((*img).numero_linhas, maior_diametro),
            tamanho_coluna_bloco = menor_divisor_k((*img).numero_colunas, maior_diametro),
            linha_atual = 0, coluna_atual = 0,
            linhas_bloco = (*img).numero_linhas / tamanho_linha_bloco, colunas_bloco = (*img).numero_colunas / tamanho_coluna_bloco,
            total_divisorias = linhas_bloco * colunas_bloco,
            divisorias_faltantes = total_divisorias;

        recebimentos * recebidos = malloc(sizeof(recebimentos) * total_divisorias);

        for(int i = 0; i < linhas_bloco; i++){
            for(int j = 0; j < colunas_bloco; j++){
                recebidos[i * colunas_bloco + j].recebido = false;
            }
        }

        for(int i = 1; i < numero_processos && divisorias_faltantes > 0; i++){
            envia_vetor(i, total_divisorias - divisorias_faltantes, &coluna_atual, &linha_atual, img, tamanho_linha_bloco, tamanho_coluna_bloco);
            divisorias_faltantes--;
        }

        int divisorias_para_iterar = divisorias_faltantes;
        int divisorias_recebidas = 0;

        for (int j = 0; j < divisorias_para_iterar; j++) {
            int indice = recebe_vetor(recebidos);
            divisorias_recebidas++;
            envia_vetor(indice, total_divisorias - divisorias_faltantes, &coluna_atual, &linha_atual, img, tamanho_linha_bloco, tamanho_coluna_bloco);
            divisorias_faltantes--;
        }

        while(divisorias_recebidas < total_divisorias){
            recebe_vetor(recebidos);
            divisorias_recebidas++;
        }

        int metade_antes = 0, total_antes = 0;
        for (int i = 0; i < total_divisorias; i++) {
            total_antes += recebidos[i].numero_estrelas_inteiras;
            metade_antes += recebidos[i].numero_estrelas_metade;
        }
        resolve_estrelas_metade(total_divisorias, recebidos, colunas_bloco);
        int total = 0;
        for (int i = 0; i < total_divisorias; i++) {
            total += recebidos[i].numero_estrelas_inteiras;
        }

        printf("\t==============================================================\n");
        printf("\t|   ESTRELAS QUE NÃO   | ESTRELAS QUE ATRAVESSAM  | TOTAL DE |\n");
        printf("\t| ATRAVESSAM PROCESSOS |        PROCESSOS         | ESTRELAS |\n");
        printf("\t==============================================================\n");
        printf("\t| %20d | %24d | %8d |\n", total_antes, metade_antes, total);
        printf("\t==============================================================\n\n");

        for (int j = 1; j < numero_processos; j++) {
            int qualquer_valor[3];
            MPI_Send(qualquer_valor, 3, MPI_INT, j, 10, MPI_COMM_WORLD);
        }

    } else {

        MPI_Status status;
        int informacoes[4];
        MPI_Recv(informacoes, 4, MPI_INT, 0, MPI_ANY_SOURCE, MPI_COMM_WORLD, &status);

        while (status.MPI_TAG != 10){

            int vetor[informacoes[1]][informacoes[2]];
            MPI_Recv(vetor, informacoes[1]*informacoes[2], MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

            int quantidade_estrelas = 0;
            int quantidades_estrelas_inteiras = 0;

            estrela * estrelas = conta_estrelas(&quantidade_estrelas, &quantidades_estrelas_inteiras, 208, informacoes, informacoes[1], informacoes[2], vetor);

            MPI_Send(&informacoes[0], 1, MPI_INT, 0, 4, MPI_COMM_WORLD);
            MPI_Send(&quantidade_estrelas, 1, MPI_INT, 0, 5, MPI_COMM_WORLD);
            int numero_estrelas_metade = quantidade_estrelas - quantidades_estrelas_inteiras;
            MPI_Send(&numero_estrelas_metade, 1, MPI_INT, 0, 6, MPI_COMM_WORLD);

            int *quantidade_estrelas_vetor = malloc(quantidade_estrelas * sizeof(int));
            for (int i = 0; i < quantidade_estrelas; i++){
                if(estrelas[i].e_borda){
                    quantidade_estrelas_vetor[i] = 1;
                } else {
                    quantidade_estrelas_vetor[i] = 0;
                }
            }

            MPI_Send(quantidade_estrelas_vetor, quantidade_estrelas, MPI_INT, 0, 7, MPI_COMM_WORLD);

            int *bordas = malloc(numero_estrelas_metade * 8 * sizeof(int));
            int cont = 0;
            for (int i = 0; i < quantidade_estrelas; i++){
                if(quantidade_estrelas_vetor[i]){
                    for(int j = 0; j < 8; j++){
                        bordas[cont * 8 + j] = estrelas[i].vetor_borda[j];
                    }
                    cont++;
                }
            }

            MPI_Send(bordas, numero_estrelas_metade * 8, MPI_INT, 0, 8, MPI_COMM_WORLD);

            free(quantidade_estrelas_vetor);
            free(bordas);
            free(estrelas);

            MPI_Recv(informacoes, 4, MPI_INT, 0, MPI_ANY_SOURCE, MPI_COMM_WORLD, &status);
        }

    }

    MPI_Finalize();

}
