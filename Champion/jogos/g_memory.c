#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "g_memory.h"

int continua = 0;

void initRandom() {
    srand(time(NULL));
}

int intUniformRnd(int a, int b) {
    return a + rand() % (b - a + 1);
}

void terminaCampeonato(int sinal, siginfo_t *info, void *context){
    continua = 1;
}

int main(int argc, char** argv) {

    int operador = 0, a = 0, b = 0, resultado = 0, pontuacao = 0, num = 0;
    char resposta [5];

    struct sigaction act;
    act.sa_sigaction = terminaCampeonato;
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    printf("BRAIN STORM\nInstrucoes:Resolva o maximo de equacoes que conseguir.\n");
    fflush(stdout);

    initRandom ();


    do{
        a = intUniformRnd(1,20);
        b = intUniformRnd(1,20);

        operador = intUniformRnd(1,4);

        switch (operador){
            case 1:
                printf ("%d + %d =", a, b);
                fflush(stdout);
                resultado = a + b;
                break;

            case 2:
                printf ("%d - %d =", a, b);
                fflush(stdout);
                resultado = a - b;
                break;

            case 3:
                printf ("%d * %d =", a, b);
                fflush(stdout);
                resultado = a * b;
                break;

            case 4:
                printf ("%d / %d =", a, b);
                fflush(stdout);
                resultado = a / b;
                break;

            default:
                break;
        }
        //Scanf nao vai confirmar esta condicao caso receba o sinal, fazendo com saia do ciclo e termine o jogo
        if(scanf ("%s",resposta) == 1){

            num = atoi(resposta);

            if (num == resultado){
                printf("Certo!\n");
                fflush(stdout);
                pontuacao ++;
            }
            else {
                printf("Errado! A resposta correta seria %d.\n",resultado);
                fflush(stdout);
            }
        }
    }while(continua != 1);

    exit(pontuacao);
}