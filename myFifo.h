#ifndef MYFIFO_H
#define MYFIFO_H

#include <semaphore.h>

#define MAX_NODES 100

typedef struct nodoProc {
    int Pi; //Usado para debug
    sem_t sem;
} nodoProc_t;

typedef struct fifoQ_s {
    int head;
    int tail;
    int recursoLivre;
    nodoProc_t nodos[MAX_NODES];
    sem_t mutex;
} FifoQT;

/* Inicializa a fila. Obs: F precisa estar alocada em shared memory. */
void init_fifoQ(FifoQT *fila);

/* Cria um nodo para o processo que chamou a funcao, e o coloca na fila. 
   Obs: A funcao nao tem mutex interno, entao eh necessario utilizar bloqueio externo ao fazer a chamada.
   Obs2: Se não houver mais espaço na fila ocorre um deadlock. */
void enfila(FifoQT* fila, int pid);

/* Retorna um ponteiro para o nodo na cabeça da fila, e move a cabeça. 
   Obs: A funcao nao tem mutex interno, entao eh necessario utilizar bloqueio externo ao fazer a chamada. */
nodoProc_t* desenfila(FifoQT* fila);

#endif