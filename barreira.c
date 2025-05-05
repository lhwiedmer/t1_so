#include <semaphore.h>
#include "barreira.h"

/* Cria a estrutura de barreira com n sendo o número de processos esperado.
   Obs: barr precisa estar alocada em shared memory. */
void init_barr( barrier_t *barr, int n ) {
    barr->maxProcNum = n;
    barr->procNumCount = 0;
    sem_init(&barr->sem, 1, 0);
    sem_init(&barr->mutex, 1, 1);
}

/* Adiciona na contagem de processos do semaforo, e se for o ultimo processo
   libera os outros. */
void process_barrier(barrier_t *barr ) {
    sem_wait(&barr->mutex);
    barr->procNumCount++;
    int n = barr->maxProcNum;
    if (barr->procNumCount == barr->maxProcNum) {
        for (int i = 0; i < n; i++) {
            sem_post(&barr->sem);
        }
        barr->procNumCount = 0;
    }
    sem_post(&barr->mutex);
    sem_wait(&barr->sem);
}