#ifndef IMAGE_UTILS_H_  
#define IMAGE_UTILS_H_

typedef struct image{
    int numero_linhas;
    int numero_colunas;
    int max_tom_cinza;
    int * pixels;
} image;

image* read_image(char *nome);

#endif