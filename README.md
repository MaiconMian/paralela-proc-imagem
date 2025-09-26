# Contagem de Estrelas em Paralelo

Este repositório contém o trabalho de **Paralela: Contagem de Estrelas**, desenvolvido em C com **MPI**.  
O principal objetivo é contar o número de estrelas de uma imagem `.pgm` de forma paralela, com um processo principal dividindo a imagem em blocos e enviando aos demais processos.  

O grande desafio do trabalho é **tratar estrelas que podem ser contadas por mais de um processo**, evitando duplicações.

---

## Como rodar

Após compilar o projeto com:

```bash
make
```
Você pode executar o programa em paralelo utilizando MPI da seguinte forma:
```bash
mpirun -np [numero_processos] --hostfile hostfile.txt contar-estrelas [nome_arquivo].pgm
```
