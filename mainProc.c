#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "barreira.h" //código da barreira
#include "myFifo.h" //código da fifo de processos

#define SHM_BARR 1234
#define SHM_FILA 5678

void inicia_uso(FifoQT * fila, int pid) {
    sem_wait(&fila->mutex); //bloqueia a mutex para garantir que um processo lida com a fila por vez

    if (sem_trywait(&fila->recurso) == 0) { //se o recurso esta livre para uso, sai da funcao
        sem_post(&fila->mutex); //desbloqueia o mutex
        return;
    }
    int myIndex = fila->tail;
    enfila(fila, pid); //enfila o processo
    sem_post(&fila->mutex); //desbloqueia o mutex
    sem_wait(&fila->nodos[myIndex].sem); //faz o processo atual esperar ate chegar sua vez
    sem_destroy(&fila->nodos[myIndex].sem); //processo saiu da espera e esse semáforo não será usado novamente, então destruimos
}

void termina_uso(FifoQT * fila) {
    sem_wait(&fila->mutex); //bloqueia a mutex para garantir que so um processo lida com a fila
    
    nodoProc_t* next = desenfila(fila);
    if (next != NULL) {
        sem_post(&next->sem); //tem gente na fila, ele vai ser o primeiro a usar
    } else {
        sem_post(&fila->recurso); //ninguem vai usar o recurso, o primeiro que aparecer pode pegar
    }
    sem_post(&fila->mutex); //desbloqueia o mutex
}   

//Retorna 1 se str eh numero e 0 se nao for 
int strEhNumero(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if ((str[i] > '9') || (str[i] < '0')) {
            return 0;
        }
    }
    return 1;
}

//argc e argv para pegar o numero de processos filhos a serem criados
int main(int argc, char** argv) {    
    if (argc != 2) {
        printf("Número de argumentos inválido na chamda de programa.\n"
                "Padrão: ./mainProc n sendo n o número de processsos a serem criados (incluindo o pai).\n");
        exit(1);
    }
    if (!strEhNumero(argv[1])) {
        printf("Argumento da chamada de programa precisa ser um inteiro.\n");
        exit(1);
    }
    int n = atoi(argv[1]); //Número de processos a ser criado
    if (n > MAX_NODES) {
        printf("Número de processos ultrapassa o máximo definido no programa.\n"
               "Esse valor pode ser mudado aumentando o valor de MAX_NODES.\n");
        exit(1);
    }
    
    int recurso = rand() % 100;

    int shmid_barr = shmget(SHM_BARR, sizeof(barrier_t), 0666|IPC_CREAT); //cria a região de memoria compartilhada
    if (shmid_barr == -1) {
        perror("shmget error");
        exit(1);
    }
    barrier_t *barr = (barrier_t *)shmat(shmid_barr, NULL, 0); // faz o attach da memória
    if (barr == (void *)-1) {
        perror("shmat error");
        exit(1);
    }

    int shmid_fila = shmget(SHM_FILA, sizeof(FifoQT), 0666|IPC_CREAT); //cria a região de memoria compartilhada
    if (shmid_fila == -1) {
        perror("shmget error");
        exit(1);
    }
    FifoQT *fila = (FifoQT *)shmat(shmid_fila, NULL, 0); // faz o attach da memória
    if (fila == (void *)-1) {
        perror("shmat error");
        exit(1);
    }

    init_fifoQ(fila);
    init_barr(barr, n);
    int Pi = 0;
    for (int i = 0; i < n-1; i++) {
        pid_t pid = fork(); //cria os filhos
        if (pid < 0) {
            perror("fork() error");
            exit(1);
        } else if (pid == 0) {
            // garante que o os filhos nao fazem fork
            Pi = i + 1;
            srand(time(NULL));
            break;
        }
    }

    printf("--Processo: %d chegando na barreira\n", Pi);
    process_barrier(barr);
    printf("--Processo: %d saindo da barreira\n", Pi);

    int uso;
    for(uso=0; uso<3; uso++ ) {
        int s = rand() % 4;
        printf( "Processo: %d Prologo: %d de %d segundos\n", Pi, uso, s);
        sleep(s);
        inicia_uso(fila, Pi);

        s = rand() % 4;
        printf("Processo: %d USO: %d por %d segundos\n", Pi, uso, s);
        sleep(s);
        termina_uso(fila);
        
        s = rand() % 4;
        printf("Processo: %d Epílogo: %d por %d segundos\n", Pi, uso, s);
        sleep(s);
    }

    printf( "--Processo: %d chegando novamente na barreira\n", Pi);
    process_barrier(barr);
    printf( "++Processo: %d saindo da barreira novamente\n", Pi);

    if (Pi == 0) { //se for pai
        for (int i = 0; i < n-1; i++) { //espera cada filho terminar
            int status;
            pid_t pid = wait(&status); //pega o status de retorno e pid
            int logic_pid = WEXITSTATUS(status);
            printf( " +++Filho de número lógico %d e pid %d terminou!\n", logic_pid, pid);
        }

        sem_destroy(&fila->mutex);
        sem_destroy(&fila->recurso);
        sem_destroy(&barr->mutex);
        sem_destroy(&barr->sem);
        shmdt(barr);
        shmdt(fila);
        shmctl(shmid_fila, IPC_RMID, NULL);
        shmctl(shmid_barr, IPC_RMID, NULL);
        return 0; //O pai retorna 0
    }


    exit(Pi);
}