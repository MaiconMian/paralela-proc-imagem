#ifndef _CONTAGEM_ESTRELAS_H_
#define _CONTAGEM_ESTRELAS_H_

typedef struct estrela{
    int indice;
    bool e_borda;
    int vetor_borda[8];
} estrela;

estrela * conta_estrelas(int * quantidade_estrelas, int * quantidades_estrelas_inteiras, int limiar, int * informacoes, int numero_linhas, int numero_colunas, int parte_imagem[numero_linhas][numero_colunas]);
bool dentro(int i, int j, int linhas, int colunas);
void inicia_estrela(estrela * estrelas, int * atual_index);
int pega_id_vizinho(int i, int j, int linhas, int colunas, int matriz[linhas][colunas], int desloc[8][2]);
void marca_vetor_borda(estrela *estrelas, int index, int i, int j, int linhas, int colunas);

#endif
