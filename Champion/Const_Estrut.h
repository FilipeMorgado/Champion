// definicao de constantes e tamanhos
#define NAME_SIZE 100
#define RESP_SIZE 300
#define FIFO_SIZE 40
#define CMD_SIZE  15
#define ELIM_SIZE 20

// definicao de frases
#define F_CMD_QUIT "Foste eliminado do Jogo!"
#define F_CMD_EXIT "O arbitro foi encerrado pelo administrador!"
#define F_CMD_KILL "Foste eliminado do Jogo pelo Administrador!"
#define F_EXISTE_JOG "O jogador ja existe"
#define F_NUM_MAX "Numero maximo de jogadores atingido"
#define F_CAMP_INI "O campeonato vai iniciar, ja nao podem ser aceites mais jogadores!"
#define F_VENCEDOR "Parabens! Foste o grande vencedor deste campeonato!"
#define F_PERDEDOR "Nao foi o vencedor do campeonato."

// definicao de fifos
#define FIFO_ARB "arb"
#define FIFO_CLI "cli%d"
#define DEBUG 1

// definicao estruturas
typedef struct mensagem mensagem;

struct mensagem {
    char nome[NAME_SIZE];
    char mensagem[NAME_SIZE];
    int pid;
};
