#include "Const_Estrut.h"
#include "cliente.h"

#include <signal.h>

int jogoIniciado = 0;

void recebeSinal(int sinal) {
    jogoIniciado = 0;
}

int main(int argc, char *argv[]) {

    int fd, fdr, fdt, n;
    char fifo[FIFO_SIZE], str[RESP_SIZE];
    mensagem msg;
    fd_set fds;
    //Tratamento de sinal
    signal(SIGUSR1, recebeSinal);
    //verifica se existe arbitro
    if (access(FIFO_ARB, F_OK) != 0) {
        fprintf(stderr, "[ERRO] Inicie o arbitro\n");
        exit(1);
    }
    msg.pid = getpid();
    sprintf(fifo, FIFO_CLI, msg.pid);

    // cria o fifo do cliente
    mkfifo(fifo, 0600);
    printf("Criei um fifo - '%s'...\n", fifo);

    // abre o fifo do arbitro para escrita
    fd = open(FIFO_ARB, O_WRONLY);
    printf("Abri o fifo do arbitro - '%s'...\n", FIFO_ARB);

    //abrir o fifo do cliente para leitura e escrita
    fdr = open(fifo, O_RDWR);
    printf("Abri o fifo do cliente'%s' \n", fifo);

    do {
        if (jogoIniciado == 0) {
            printf("Bem-vindo!!\n");
            fflush(stdout);
            do {
                //solicita username ao utilizador
                printf("Por favor digite o seu username:\n");
                fflush(stdout);
                scanf("%s", msg.nome);

                if (access(FIFO_ARB, F_OK) != 0) {
                    fprintf(stderr, "[ERRO] Inicie o arbitro\n");
                    unlink(fifo); remove(fifo);
                    exit(1);
                }

                //envia mensagem ao arbitro
                n = write(fd, &msg, sizeof(mensagem));

                //recebe resposta do arbitro
                memset(str, 0, RESP_SIZE);
                n = read(fdr, str, RESP_SIZE);
                printf("%s\n", str);

            } while (strcmp(str, F_EXISTE_JOG) == 0 && strcmp(str, F_NUM_MAX) != 0 && strcmp(str, F_CAMP_INI) != 0 );
        }
        //Evita entrar no if seguinte
        if(strcmp(str, F_NUM_MAX) == 0 || strcmp(str, F_CAMP_INI) == 0)
            break;

        //Feita a conexao ao fifo em que vai utilizar para escrita de mensagens para o arbitro
        if (access(msg.nome, F_OK) != 0) {
                fprintf(stderr, "[ERRO] Erro ao aceder ao fifo do jogador %s\n", msg.nome);
                //fecha fifo do arbitro
                close(fd);
                close(fdr);
                //apaga fifo do cliente
                unlink(fifo);
                remove(fifo);
                exit(1);
        }
        //Abertura do fifo para escrita
        fdt = open(msg.nome, O_RDWR);
        printf("Abri o fifo - '%s'...\n", msg.nome);

        if (strcmp(str, F_NUM_MAX) != 0 && strcmp(str, F_CAMP_INI) != 0) {
            jogoIniciado = 1;
            printf("COMANDO:\n");
            fflush(stdout);
            do {
                fflush(stdout);
                FD_ZERO(&fds);
                FD_SET(0, &fds);    /* Responsavel pelo STDIN */
                FD_SET(fdr, &fds);   /* Responsavel pelo NamedPipe */

                int r = select(fdr + 1, &fds, NULL, NULL, NULL);

                if (r > 0 && FD_ISSET(0, &fds)) {  /* Le dados do stdin */

                    //solicita comandos ao utilizador
                    scanf("%s", msg.mensagem);

                    //envia mensagem ao arbitro
                    n = write(fdt, msg.mensagem, strlen(msg.mensagem));
                    printf("COMANDO:");
                    fflush(stdout);
                } else if (r > 0 && FD_ISSET(fdr, &fds)) { /* Le respostas do arbitro */

                    //recebe resposta do arbitro
                    memset(str, 0, RESP_SIZE);
                    n = read(fdr, str, RESP_SIZE);
                    printf("%s\n", str);

                    if (strcmp(str, F_CMD_QUIT) == 0 || strcmp(str, F_CMD_EXIT) == 0 || strcmp(str, F_CMD_KILL) == 0 || strcmp(str, F_VENCEDOR) == 0) {
                        jogoIniciado = 0;
                        break;
                        //sai do ciclo quando a STR é uma das mencionadas acima.
                    }
                }
            } while (jogoIniciado == 1);
        }
        //Este read é feito para ler a ultima mensagem recebida por parte do arbitro, a sua respetiva pontuacao.
        if (jogoIniciado == 0 && !strcmp(str, F_VENCEDOR) == 0 && !strcmp(str, F_CMD_KILL) == 0 && !strcmp(str, F_CMD_EXIT) == 0 && !strcmp(str, F_PERDEDOR) == 0 && !strcmp(str, F_CMD_QUIT) == 0) {
            memset(str, 0, RESP_SIZE);
            n = read(fdr, str, RESP_SIZE);
            printf("%s\n", str);
        }
    } while (strcmp(str, F_CMD_QUIT) != 0 && strcmp(str, F_CMD_EXIT) != 0 && strcmp(str, F_CMD_KILL) != 0);
    //fecha fifo do arbitro
    close(fd);
    close(fdr);

    //apaga fifo do cliente
     unlink(fifo); remove(fifo);

    exit(0);
}
