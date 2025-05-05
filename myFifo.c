#include <stdlib.h>
#include <stdio.h>
#include "myFifo.h"

void init_fifoQ(FifoQT *fila) {
    fila->head = 0;
    fila->tail = 0;
    sem_init(&fila->mutex, 1, 1);
    sem_init(&fila->recurso, 1, 1);
}

void enfila(FifoQT* fila, int pid) {
    if ((fila->tail + 1) % MAX_NODES == fila->head) {
        printf("Fila de processos encheu, terminando o programa\n");
        return;
    }
    fila->nodos[fila->tail].pid = pid;
    sem_init(&fila->nodos[fila->tail].sem, 1, 0);
    fila->tail = (fila->tail + 1) % MAX_NODES;
}

nodoProc_t* desenfila(FifoQT* fila) {
    if (fila->head == fila->tail) {
        return NULL;
    }
    nodoProc_t* nodo = &fila->nodos[fila->head];
    fila->head = (fila->head + 1) % MAX_NODES;
    return nodo;
}