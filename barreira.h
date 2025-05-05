#ifndef BARREIRA_H
#define BARREIRA_H

#include <semaphore.h>

typedef struct barrier_s {
    int maxProcNum;
    int procNumCount;
    sem_t sem;
    sem_t mutex;
} barrier_t;          // tipo barrier_t 
    

/* Cria a estrutura de barreira com n sendo o número de processos esperado. */
void init_barr( barrier_t *barr, int n );


/* Adiciona na contagem de processos da semaforo, e se for o ultimo processo
   libera os outros. */
void process_barrier( barrier_t *barr );

#endif