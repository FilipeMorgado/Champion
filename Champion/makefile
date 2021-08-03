all:  cliente arbitro jogo1 jogo2

cliente: cliente.o
	gcc cliente.o -o cliente

cliente.o: cliente.c cliente.h Const_Estrut.h
	gcc -c cliente.c

arbitro: arbitro.c arbitro.h Const_Estrut.h
	gcc -c arbitro.c
	gcc arbitro.c -o arbitro -lpthread

jogo1: ./jogos/g_memory.c ./jogos/g_memory.h
	gcc -c ./jogos/g_memory.c -o ./jogos/g_memory.o
	gcc ./jogos/g_memory.c -o jogos/g_memory

jogo2: ./jogos/g_perguntas.c ./jogos/g_perguntas.h
	gcc -c ./jogos/g_perguntas.c -o ./jogos/g_perguntas.o
	gcc ./jogos/g_perguntas.c -o jogos/g_perguntas

clean:
# -f para ignorar erros quando algum ficheiro nao existe
	rm -f arbitro.o cliente.o ./jogos/g_memory.o ./jogos/g_perguntas.o arbitro cliente ./jogos/g_memory ./jogos/g_perguntas

