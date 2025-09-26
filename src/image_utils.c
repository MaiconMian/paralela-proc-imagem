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

image* read_image(char *nome){

    image *img = malloc(sizeof (*img));
    FILE *file = fopen(nome, "r");
    
    if (!file){
        printf("\n\tErro ao abrir o .pgm!\n");
        printf("\tCertifique-se de escrever da forma correta:\n");
        printf("\t[nome_arquivo].pgm\n\n");
        exit(1);
    };

    char line[100];
    fgets(line, sizeof(line), file); 
    fgets(line, sizeof(line), file); 
    sscanf(line, "%d %d", &(*img).numero_linhas, &(*img).numero_colunas);
    fgets(line, sizeof(line), file);
    sscanf(line, "%d", &(*img).max_tom_cinza);

    (*img).pixels = malloc(sizeof(int) * (*img).numero_linhas * (*img).numero_colunas);

    for (int i = 0; i < (*img).numero_linhas * (*img).numero_colunas; i++){
        int k;
        fscanf(file, "%d", &k);
        (*img).pixels[i] = k;
    }

    fclose(file);
    return img;

}