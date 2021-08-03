#ifndef ARBITRO_H
#define ARBITRO_H
#include "Const_Estrut.h"

//includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/wait.h>

// definicao de erros
#define ERRO_UTILIZACAO_INVALIDA -1

// definicao de constantes
#define NUM_ARGS 3
#define JOGO_SIZE 30

typedef struct jogador jogador, *pjogador;

struct jogador {
    char nome [NAME_SIZE];
    int pid;
    int pontuacao;
    int desiste;
    int suspende;
    int pidJogo;
    char jogo [JOGO_SIZE];
    pthread_t threadJog;
    pjogador prox;
};

typedef struct campeonato campeonato, *pcampeonato;
struct campeonato {
    pjogador cliente;
    int num_jogadores;
};

#endif