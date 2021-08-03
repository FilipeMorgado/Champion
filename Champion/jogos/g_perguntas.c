#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "g_perguntas.h"

int continua = 0;

void terminaCampeonato(int sinal, siginfo_t *info, void *context) {
    continua = 1;
}

int main(int argc, char **argv) {
    int pontuacao = 0, i = 0;
    char resposta, resultado;

    struct sigaction act;
    act.sa_sigaction = terminaCampeonato;
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    printf("QUEM NAO QUER SER MILIONARIO\nInstrucoes:Responda corretamenta as seguintes questoes.\n");
    fflush(stdout);
    do {
        if(i == 11)
            i = 0;
            switch(i){
            case 0:
                printf("\nQuem tem o maior numero de golos realistas no futebol?\n");
                fflush(stdout);
                printf("A: Pele->40323\tB: Cristiano Ronaldo->776\nC: Romario->772\tD: Puskas->746\n");
                fflush(stdout);
                resultado = 'b';
                break;
            case 1:
                printf("\nQue famosa banda pop americana era originalmente chamada de ‘Kara’s Flowers’?\n");
                fflush(stdout);
                printf("A: Imagine Dragons\tB: Maroon 5\nC: D'ZRT\tD: Sancho II\n");
                fflush(stdout);
                resultado = 'b';
                break;
            case 2:
                printf("\nQual foi a batalha mais mortal da Segunda Guerra Mundial?\n");
                fflush(stdout);
                printf("A: Moscovo\tB: El Alamein\nC: Stalingrado\tD: Normandia\n");
                fflush(stdout);
                resultado = 'c';
                break;
            case 3:
                printf("\nQual é o valor de π?\n");
                fflush(stdout);
                printf("A: 3.15\tB: 2.14123\nC: 3.1415927\tD: 14,3654\n");
                fflush(stdout);
                resultado = 'c';
                break;
            case 4:
                printf("\nQuantos fusos horários tem a Rússia?\n");
                fflush(stdout);
                printf("A: 2\tB: 6\nC: 11\tD: 5\n");
                fflush(stdout);
                resultado = 'c';
            break;
            case 5:
                printf("\nQuantas riscas tem a bandeira dos EUA?\n");
                fflush(stdout);
                printf("A: 11\tB: 13\nC: 7\tD: 6\n");
                fflush(stdout);
                resultado = 'a';
                break;
            case 6:
                printf("\nQue país tem mais ilhas no mundo?\n");
                fflush(stdout);
                printf("A: Portugal\tB: Austria\nC: Suecia\tD: Noruega\n");
                fflush(stdout);
                resultado = 'c';
                break;
            case 7:
                printf("\nQuando é que abriu o metropolitano de Londres?\n");
                fflush(stdout);
                printf("A: 1863\tB: 1904\nC: 1833\tD: 1923\n");
                fflush(stdout);
                resultado = 'a';
                break;
            case 8:
                printf("\nQuantas teclas tem um piano clássico?\n");
                fflush(stdout);
                printf("A: 45\tB: 52\nC: 66\tD: 88\n");
                fflush(stdout);
                resultado = 'd';
            break;
            case 9:
                printf("\nQue piloto de Fórmula 1 ganhou mais corridas?\n");
                fflush(stdout);
                printf("A: Schumacher\tB: Lewis Hamilton\nC: Vettel\tD: Lance Stroll\n");
                fflush(stdout);
                resultado = 'b';
            break;
            case 10:
                printf("\nQual era o nome original da marca de roupa Nike? \n");
                fflush(stdout);
                printf("A: Mike\tB: Blue Ribbon Sports\nC: Nykee\tD: Red Tennis Sports\n");
                fflush(stdout);
                resultado = 'b';
                break;
            default:
            break;
            }
            //Scanf nao vai confirmar esta condicao caso receba o sinal, fazendo com saia do ciclo e termine o jogo
        if (scanf(" %c", &resposta) == 1) {
            if (resposta == resultado) {
                printf("Certo!\n");
                fflush(stdout);
                pontuacao++;
            } else
                printf("Errado! A resposta correta seria %c.\n", resultado);
        }
        i++;
    } while (continua != 1);

    exit(pontuacao);
}