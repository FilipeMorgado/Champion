#include "arbitro.h"

int tempoEsp, durCamp, maxJogadores = 0, numJogos = 0, continua_atender_clientes = 1, terminaCamp = 0, iniciaCampeonato = 0, contador = 0;
char **jogos = NULL, *gamedir;
campeonato camp;

pthread_mutex_t mutex;

void initRandom() {
    srand(time(NULL));
}
//Funcao para geracao de aleatorios
int intUniformRnd(int a, int b) {
    return a + rand() % (b - a + 1);
}

void libertaClientes(){
    pjogador temp;
    while(camp.cliente != NULL) {
        temp = camp.cliente;
        camp.cliente = camp.cliente->prox;
        free(temp);
    }
    camp.num_jogadores = 0;
    camp.cliente = NULL;
}

pjogador procuraJogador(char *nome) {
    pjogador aux = camp.cliente;
    while(aux != NULL){
        if (strcmp(nome, aux->nome) == 0) {
            return aux;
        }
        aux = aux->prox;
    }
    return NULL;
}

void extraiComando(char *comandoMsg, char *comando) {
    int i;
    for (i = 1; i < strlen(comandoMsg); i++) {
        comando[i - 1] = comandoMsg[i];
    }
    comando[i - 1] = '\0';
}

void enviaMensagemFifoCli(char *resposta, int pid) {
    char fifo[40];
    sprintf(fifo, FIFO_CLI, pid);
    //envia resposta ao cliente
    int fdr = open(fifo, O_WRONLY);
    write(fdr, resposta, strlen(resposta));
    close(fdr);
}

//Verifica se existe um jogador e caso exista retorna o seu pid
int existeJogador(char *nome) {
    pjogador aux = camp.cliente;
    while(aux != NULL){
        if (strcmp(nome, aux->nome) == 0) {
            return aux->pid;
        }
        aux = aux->prox;
    }
    return 0;
}

int verificaExistePid(int pid) {
    pjogador aux = camp.cliente;
    while(aux != NULL){
        if (pid == aux->pid) {
            return aux->pid;
        }
        aux = aux->prox;
    }
    return 0;
}

pjogador adicionajogadoresCampeonato(mensagem msg) {

    pjogador aux = camp.cliente;
    pjogador novoJogador = NULL;

    //Verifica se ja existe algum jogador com o mesmo nome
    if (!existeJogador(msg.nome)) {
        novoJogador = malloc(sizeof(jogador));
        //Verifica se o malloc ocorreu corretamente
        if (novoJogador == NULL) {
            printf("Erro na alocacao de memoria\n");
        }
        //Inicializa os dados
        novoJogador->prox = NULL;
        strcpy(novoJogador->nome, msg.nome);
        novoJogador->pid = msg.pid;
        int randNum = intUniformRnd(0, numJogos - 1);
        strcpy(novoJogador->jogo, jogos[randNum]);
        //Caso seja o primeiro jogador
        if (camp.cliente == NULL) {
            camp.cliente = novoJogador;
        } else {
            //Atualiza ponteiros
            aux = camp.cliente;
            while (aux->prox != NULL) {
                aux = aux->prox;
            }
            aux->prox = novoJogador;
        }
        //Incremente numero de jogadores
        camp.num_jogadores++;
    }
    return novoJogador;
}

int eliminaJogadorCampeonato(char *nome) {
    int i;
    union sigval value;
    value.sival_int = 0;
    pjogador atual = camp.cliente;
    pjogador prev = NULL;

    //Verifica se o jogador existe
    pjogador jogadorEliminar = procuraJogador(nome);
    if(jogadorEliminar == NULL){
        return 0;
    }
    //Caso nao tenha nenhum jogador, retorna
    if(camp.num_jogadores == 0){
        camp.cliente = NULL;
        return 1;
    }
    else{
        //Procura o jogador na lista
        while (atual != NULL && strcmp(atual->nome,jogadorEliminar->nome) != 0) {
            prev = atual;
            atual = atual->prox;
        }
        //Se nao existir, retorna 0
        if (atual == NULL) {
            return 0;
        }
        //Caso o campeonato esteja a decorrer, manda sinal ao jogo para terminar
        if (iniciaCampeonato == 1) {
            sigqueue(jogadorEliminar->pidJogo, SIGUSR1, value);
        }
        //Caso o cliente seja o primeiro da lista, atualiza ponteiro
        if(atual == camp.cliente) {
            camp.cliente = camp.cliente->prox;
            camp.num_jogadores--;
            return 1;
        }
        //Caso geral, ponteiro anterior, aponta para o proximo, removendo o jogador da lista
        prev->prox = atual->prox;
        camp.num_jogadores--;
    }
   return 1;
}

int cmdElimina(char *cmd) {
    char nome_elim[ELIM_SIZE];

    extraiComando(cmd, nome_elim);
    int pid = existeJogador(nome_elim); //PID do jogador
    if (eliminaJogadorCampeonato(nome_elim)) {
        enviaMensagemFifoCli(F_CMD_KILL, pid);
        return 1;
    }
    return 0;
}

void determinaVencedor() {

    int maior = 0, pidVencedor = 0;
    pjogador aux = camp.cliente;

    if(camp.num_jogadores == 1) {
        enviaMensagemFifoCli("Parabens! Foste o grande vencedor deste campeonato!", aux->pid);
        return;
    }

    while(aux != NULL){
        if (aux->pontuacao > maior) {
            pidVencedor = aux->pid;
            maior = aux->pontuacao;
        }
        aux = aux->prox;
    }

        aux = camp.cliente;
        while (aux != NULL) {
            if (aux->pid == pidVencedor)
                enviaMensagemFifoCli("Parabens! Foste o grande vencedor deste campeonato!", pidVencedor);
            else {
                enviaMensagemFifoCli("Nao foi o vencedor do campeonato.", aux->pid);
            }
            aux = aux->prox;
        }
}

void finalizaCampeonato() {

    pjogador aux = camp.cliente;
    //Inicio de termino de campeonato
    terminaCamp = 1;
    if(iniciaCampeonato == 1) {
        printf("O campeonato vai terminar\n");

        //Espera que as threads dos jogadores terminem
        while (aux != NULL) {
            //Envia sinal a thread do jogador, pois encontra-se "presa" no select
            pthread_kill(aux->threadJog, SIGUSR1);
            //Aguarda o termino da thread do jogador, para poder determinar o vencedor com a pontuacao atualizada
            pthread_join(aux->threadJog, NULL);
            aux = aux->prox;
        }
        //Determina o vencedor
        determinaVencedor();
        iniciaCampeonato = 0;

        //Liberta Clientes
        libertaClientes();
    }
}

void cmdExit() {
    pjogador aux = camp.cliente;

    while(aux != NULL) {
        enviaMensagemFifoCli(F_CMD_EXIT, aux->pid);
        aux = aux->prox;
    }

    //Liberta Clientes caso existam
    if(camp.num_jogadores > 0)
        libertaClientes();
    free(jogos);

    //unlink e remove do fifo do arbitro
    unlink(FIFO_ARB); remove(FIFO_ARB);

    exit(1);
}

void listaJogadores() {
    int i;
    pjogador aux = camp.cliente;

    if (camp.num_jogadores == 0) {
        printf("Nao ha jogadores\n");
        return;
    }

    printf("\nListagem de jogadores existentes:\n");

    while(aux != NULL) {
        printf("Nome do jogador: %s || Jogo: %s\n", aux->nome, aux->jogo);
        aux = aux->prox;
    }
}

char **obtemJogos(int *numJogos, char *gamedir) {

    DIR *d = NULL;
    struct dirent *dir = NULL;
    char **jogos = NULL;
    char *ext;

    d = opendir(gamedir);
    if (d == NULL) {
        fprintf(stderr, "[ERRO] Nao foi possivel aceder a diretoria %s\n", gamedir);
    }
    while ((dir = readdir(d)) != NULL) {
        ext = strrchr(dir->d_name, '.');
        if (dir->d_name[0] == 'g' && dir->d_name[1] == '_' && !ext) {
            jogos = realloc(jogos, sizeof(*jogos) * (*numJogos + 1));
            jogos[(*numJogos)++] = strdup(dir->d_name);
        }
    }
    closedir(d);

    return jogos;
}

void mostraJogos(int numJogos) {
    for (int i = 0; i < numJogos; i++) {
        printf("Jogo %d: %s \n", i + 1, jogos[i]);
    }
}

//Sinal recebido para avancar no TrataJogador
void enviaSinal(int sinal) {
}
//Sinal recebido para terminar thread de atendimento de Jogadores
void termina(int sinal) {
}

void cmdRetoma(char *cmd) {
    char nome_susp[NAME_SIZE];

    extraiComando(cmd, nome_susp);

    int pid = existeJogador(nome_susp); //PID do jogador
    if (pid != 0) {
        pjogador jogador = procuraJogador(nome_susp);
        if (jogador->suspende == 1) {
            jogador->suspende = 0;
            enviaMensagemFifoCli("As comunicacoes foram retomadas !\n", pid);
        } else
            printf("O jogador ja se encontra a comunicar com o jogo.\n");

    } else {
        printf("Nao existe o jogador '%s' no campeonato\n", nome_susp);
    }
}

void cmdSuspende(char *cmd) {
    char nome_susp[NAME_SIZE];

    extraiComando(cmd, nome_susp);

    int pid = existeJogador(nome_susp); //PID do jogador
    if (pid != 0) {
        pjogador jogador = procuraJogador(nome_susp);
        if (jogador->suspende == 0) {
            jogador->suspende = 1;
            enviaMensagemFifoCli("As comunicacoes foram suspendidas !\n", pid);
        } else
            printf("As comunicacoes com o jogador ja se encontram suspensas.\n");

    } else {
        printf("Nao existe o jogador '%s' no campeonato\n", nome_susp);
    }
}


void *temporizacao(void *dados) {
    terminaCamp = 0;

    //Vai aguardar o tempo definido para aceitar os jogadores para participar no campeonato
    sleep(tempoEsp);
    printf("Vai iniciar um novo campeonato.\n");

    //Informa os clientes que o campeonato vai comecar
    pjogador aux = camp.cliente;
    while(aux != NULL) {
        if (aux->suspende == 0 && camp.num_jogadores > 1)
            enviaMensagemFifoCli("O campeonato vai comecar", aux->pid);
        aux = aux->prox;
    }
    //Inicia Campeonato
    iniciaCampeonato = 1;

    pthread_mutex_lock(&mutex);
    //Envia sinal a thread do jogador, para que possa sair do primeiro read no qual se encontra bloquado,
    //para de seguida poder iniciar os jogos e começar o jogo entre jogador e jogo
    aux = camp.cliente;
    while(aux != NULL) {
        pthread_kill(aux->threadJog, SIGUSR1);
        aux = aux->prox;
    }
    pthread_mutex_unlock(&mutex);

    //Fica a aguardar que o tempo de duração do campeonato termine, para finalizar o campeonato.
    sleep(durCamp);

    //Finaliza Campeonato
    finalizaCampeonato();
}

void processaComandoJogador(pjogador jog, char *comando, int canal) {
    int pid = jog->pid;
    char resposta[RESP_SIZE];

    if (comando[0] == '#') {
        char cmdCli[CMD_SIZE];
        extraiComando(comando, cmdCli);
        if (strcmp(cmdCli, "quit") == 0) {
            if (eliminaJogadorCampeonato(jog->nome)) {
                strcpy(resposta, F_CMD_QUIT);
            } else {
                strcpy(resposta, "Nao foi possivel eliminar do Jogo!");
            }
        } else if (strcmp(cmdCli, "mygame") == 0) {
            strcpy(resposta, jog->jogo);
        } else {
            strcpy(resposta, "Comando Invalido");
        }
        enviaMensagemFifoCli(resposta, pid);
    } else {//Caso nao seja um comando, analisa se o jogo esta aberto ou nao
        //Caso esteja com o campeonato em aberto e o cliente nao esteja suspendido, escreve no jogo
        if (canal != -1 && jog->suspende == 0) {
            strcat(comando, "\n");
            write(canal, comando, strlen(comando));
        } else { //Caso contrario, nao é um comando predefinido nem o jogo se encontra aberto
            enviaMensagemFifoCli("Comando Invalido", pid);
        }
    }
}

void *trataJogador(void *dados) {

    int fd_cli, n, resultado, fpid, fd, estado;
    char comando[RESP_SIZE], resposta[RESP_SIZE];
    pjogador jog = (pjogador)dados;

    //Declaracao de dados para envio de sinal ao cliente, para avisar de termino de campeonato
    union sigval value;
    value.sival_int = 5;
    //Criacao de pipes anonimos
    int canal_le_jogo_arb[2], canal_escreve_arb_jogo[2];
    //Abertura de fifo de leitura de dados escritos pelo cliente
    printf("Foi aberto o fifo %s\n", jog->nome);
    fd_cli = open(jog->nome, O_RDONLY);
    //Declaracao de sinal que vai ser recebido aquando inicio de campeonato, ira permitir
    //saltar o read no qual ficou 'preso' a aguardar inicio de campeonato
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = enviaSinal;
    sigaction(SIGUSR1, &act, NULL);

    fd_set fds;

    //recebe os comandos do jogador antes de iniciar o campeonato, tratando-os respetivamente
    do {
        memset(comando, 0, RESP_SIZE);
        //O sinal é enviado para que este read possa ser ultrapassado, para sair do ciclo do while e começar o tratamento do campeonato
        n = read(fd_cli, comando, RESP_SIZE);

        //acesso a informação partilhada, MUTEX
        pthread_mutex_lock(&mutex);
        //Caso receba dados no read por parte do cliente, vai verificar se é algum comando
        if (n > 0) {
            processaComandoJogador(jog, comando, -1);
        }
        pthread_mutex_unlock(&mutex);

    } while (jog->desiste == 0 && !iniciaCampeonato);// continua && nao inicia o campeonato

    //so comunica com o jogo depois de iniciar o campeonato e nao ter desistido
    if (jog->desiste == 0 && iniciaCampeonato == 1) {
        char strJogo[100];
        strcpy(strJogo, gamedir);
        if (gamedir[strlen(gamedir) - 1] != '/')
            strcat(strJogo, "/");
        strcat(strJogo, jog->jogo);
        close(fd_cli);
        fd_cli = open(jog->nome, O_RDONLY);

        //cria canal de comunicacao
        pipe(canal_le_jogo_arb);
        pipe(canal_escreve_arb_jogo);

        //Criacao de processo filho
        fpid = fork();
        jog->pidJogo = fpid;

        if (fpid == 0) {
            //redirecionamento dos dados lidos do jogo (printfs) para o pipe(arbitro) em vez do ecra
            close(canal_le_jogo_arb[0]); //fecha canal de leitura do pipe, nao vai ser usado
            close(1);                    //fechar canal de leitura (stdout), deixou de estar  ligado ao monitor
            dup(canal_le_jogo_arb[1]);   //duplica canal de escrita, vai para a 1º posicao disponivel, (canal(1))
            close(canal_le_jogo_arb[1]); //fecha canal de escrita "original" pois ja nao e usado
            //Apos redirecionamento, jogador nao escreve no stdout, mas sim no pipe canal_le_jogo

            //Redirecionamento dos dados a serem escritos no jogo (scanf's) para o pipe (arbitro)
            close(canal_escreve_arb_jogo[1]); //fecha canal de escrita do pipe, nao vai ser usado
            close(0);                         //fechar canal de escrita (stdin), deixou de estar  ligado ao monitor
            dup(canal_escreve_arb_jogo[0]);   //duplica canal de leitura, vai para a 1º posicao disponivel, (canal(0))
            close(canal_escreve_arb_jogo[0]); //fecha canal de leitura "original" pois ja nao e usado

            //Executa o jogo atribuido pelo arbitro
            execl(strJogo, strJogo, NULL);
            printf("Erro a executar. Processo filho com pid: %d\n", fpid);
            cmdExit();
        }
        //Fecha canais
        close(canal_le_jogo_arb[1]);
        close(canal_escreve_arb_jogo[0]);

        fflush(stdout);
        do {
            FD_ZERO(&fds);
            FD_SET(fd_cli, &fds);  /* Responsavel pelo jogador */
            FD_SET(canal_le_jogo_arb[0], &fds); /* Responsavel pelo jogo */
            if (camp.num_jogadores > 1) {
                //Monotorizar descritores //Bloqueia ate receber informacao no fd_cli ou canal_le_jogo_arb
                resultado = select(canal_le_jogo_arb[0] + 1, &fds, NULL, NULL, NULL);

                //Caso receba informacao por parte do jogador, ativa este if
                if (resultado > 0 && FD_ISSET(fd_cli, &fds)) {
                    memset(comando, 0, RESP_SIZE);
                    n = read(fd_cli, comando, RESP_SIZE);
                    if (n > 0) { //Caso faça o read com sucesso
                        processaComandoJogador(jog, comando, canal_escreve_arb_jogo[1]);
                    }
                } //Caso receba informacao do pipe anonimo do jogo
                else if (jog->suspende == 0) {
                    //Caso receba informacao do pipe anonimo do jogo
                    if (resultado > 0 && FD_ISSET(canal_le_jogo_arb[0], &fds)) {
                        memset(comando, 0, 100);
                        n = read(canal_le_jogo_arb[0], comando, RESP_SIZE);
                        comando[n] = '\0';
                        enviaMensagemFifoCli(comando, jog->pid);
                    }
                }
            } else {
                finalizaCampeonato();
            }
        } while (terminaCamp == 0 && jog->desiste == 0);

        //Vai matar os jogos apenas caso o campeonato tenha começado
        if(iniciaCampeonato == 1)
            //Envia sinal para matar o jogo
            sigqueue(jog->pidJogo, SIGUSR1, value);

        //Aguarda pelo termino do jogo para receber a sua pontuacao
        waitpid(fpid, &estado, 0);
        if (WIFEXITED(estado)) {
            char buffer[100];
            //Guarda a pontuacao do jogador no seu campo da estrutura "pontuacao"
            jog->pontuacao = WEXITSTATUS(estado);
            //Vai enviar uma mensagem ao cliente a informar do término do jogo e a respetiva pontuação
            sprintf(buffer, "Jogo Terminado! Pontuacao obtida:%d", WEXITSTATUS(estado));
            enviaMensagemFifoCli(buffer, jog->pid);
            //Envia sinal ao jogador para que a variavel global do jogador(cliente) "JogoIniciado" mude, encerrando o jogo do lado cliente
            kill(jog->pid, SIGUSR1);
        }
    }

    //Fecha
    close(fd_cli);
    // unlink e remove do namedPipe
    unlink(jog->nome); remove(jog->nome);
    close(canal_le_jogo_arb[0]);
    close(canal_escreve_arb_jogo[1]);
    //Caso o jogador desista, elimina da lista
    if (jog->desiste == 1) {
        free(jog);
    }
    pthread_exit(NULL);
}

void *atendeClientes(void *dados) {

    char resposta[RESP_SIZE];
    mensagem msg;

    int fd, n;
    pthread_t temporizador;

    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = termina;
    sigaction(SIGUSR1, &act, NULL);

    //verifica se existe fifo arbitro, se nao existir, cria fifo do arbitro
    if (access(FIFO_ARB, F_OK) != 0) {
        mkfifo(FIFO_ARB, 0600);
        printf("Criei um fifo...\n");
    }

    //abre o fifo do arbitro para leitura e escrita
    fd = open(FIFO_ARB, O_RDONLY);
    printf("Abri o fifo... '%s' \n", FIFO_ARB);

    do {
        n = read(fd, &msg, sizeof(mensagem));
        if (n == sizeof(mensagem)) {
            //lock caso seja feito leitura correta de informacao
            pthread_mutex_lock(&mutex);

            if (!verificaExistePid(msg.pid)) {
                if (camp.num_jogadores == maxJogadores) {
                    printf("O campeonato atingiu o numero maximo de jogadores: %d!\n", maxJogadores);
                    strcpy(resposta, F_NUM_MAX);
                } else if (iniciaCampeonato == 1) {
                    printf("O campeonato ja iniciou, volte mais tarde!\n");
                    strcpy(resposta, "Ja se encontra a decorrer um campeonato. Por favor, volte mais tarde!");
                } else {
                    //adiciona novo jogador
                    pjogador novoJog = adicionajogadoresCampeonato(msg);
                    if (novoJog != NULL) {
                        strcpy(resposta, "Jogador adicionado.");
                        printf("O cliente %d ligou-se ao servidor.\n", msg.pid);
                        mkfifo(novoJog->nome, 0600);

                        if ((pthread_create(&novoJog->threadJog, NULL, trataJogador, novoJog)) != 0) {
                            printf("[Erro] Nao foi possivel criar a thread para o jogador adicionado.\n");
                            cmdExit();
                        }
                    } else {
                        strcpy(resposta, F_EXISTE_JOG);
                    }
                }
                enviaMensagemFifoCli(resposta, msg.pid);
                if (camp.num_jogadores == 2 && strcmp(resposta, "Jogador adicionado.") == 0) {
                    printf("Foram adicionados dois jogadores ao campeonato, temporizador de %d segundos ativado!\n\n",tempoEsp);
                    if ((pthread_create(&temporizador, NULL, temporizacao, NULL)) != 0) {
                        printf("[Erro] Nao foi possivel criar a thread de temporizacao.\n");
                        cmdExit();
                    }
                }
            }
            pthread_mutex_unlock(&mutex);
        }
    } while (continua_atender_clientes);

    //Unlink e remove do fifo do arbitro
    unlink(FIFO_ARB); remove(FIFO_ARB);
    close(fd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    /* Variaveis */
    char *maxplayer, cmd[CMD_SIZE];

    camp.cliente = NULL;
    camp.num_jogadores = 0;

    pthread_t atendeCli;

    if (argc != NUM_ARGS) {
        printf("[ERRO] Numero de argumentos: Esperado -> ./arbitro <Duracao do Campeonato> <Tempo de Espera>\n");
        exit(ERRO_UTILIZACAO_INVALIDA);
    }

    durCamp = atoi(argv[1]);
    tempoEsp = atoi(argv[2]);

    if (durCamp == 0 || tempoEsp == 0) {
        printf("[ERRO] A duracao do campeonato e tempo de espera devem ser números superiores a 0.\n");
        exit(ERRO_UTILIZACAO_INVALIDA);
    }

    printf("ARGV[1] Duracao do Campeonato: %d\n", durCamp);
    printf("ARGV[2] Tempo de Espera: %d\n\n", tempoEsp);

    printf("Variáveis de Ambiente definidas:\n");
    gamedir = getenv("GAMEDIR");

    if (gamedir != NULL) {
        printf("\nA diretoria e: %s\n", gamedir);
    } else {
        gamedir = "./jogos";
        printf("\nDiretoria nao definida. Foi utilizada diretoria base %s\n", gamedir);
    }

    maxplayer = getenv("MAXPLAYER");

    if (maxplayer != NULL) {
        printf("Numero maximo de jogadores: %s \n\n", maxplayer);
        sscanf(maxplayer, "%d", &maxJogadores);
    } else {
        printf("Numero maximo de jogadores nao definido.\n\n");
        exit(1);
    }
    if (maxJogadores > 30)
        printf("Numero maximo de jogadores nao pode ser superior a 30.\n");

    //Vai ser iniciado para preencher o array de jogos com os jogos que existem
    jogos = obtemJogos(&numJogos, gamedir);

    //É feita a criação da thread que vai tratar de atender os clientes
    if ((pthread_create(&atendeCli, NULL, atendeClientes, NULL)) != 0) {
        printf("[Erro] Nao foi possivel criar a thread para atender clientes.\n");
        exit(1);
    }

    //Atende Administrador
    do {
        printf("COMANDO: ");
        fflush(stdout);
        scanf("%s", cmd);
        printf("O administrador pediu o comando '%s'\n", cmd);

        pthread_mutex_lock(&mutex);
        //comando players
        if (strcmp(cmd, "players") == 0) {
            listaJogadores();
        }
            //comando k
        else if (cmd[0] == 'k') {
            cmdElimina(cmd);
        }
            //comando para suspender o jogo do cliente
        else if (cmd[0] == 's') {
            cmdSuspende(cmd);
        }
            //comando para retomar o jogo do cliente
        else if (cmd[0] == 'r') {
            cmdRetoma(cmd);
        }
            //comando jogos
        else if (strcmp(cmd, "jogos") == 0) {
            mostraJogos(numJogos);
        }
            //comando exit
        else if (strcmp(cmd, "exit") == 0) {
            cmdExit();
        }
            //comando end
        else if (strcmp(cmd, "end") == 0) {
            if(iniciaCampeonato == 1)
                finalizaCampeonato();
            else
                printf("Tem que iniciar o campeonato primeiro!\n");
        } else {
            printf("Comando Invalido\n");
        }
        pthread_mutex_unlock(&mutex);
    } while (1);
}


