# Makefile do BiblioHash
# Comandos principais:
#   make              compila o programa (bibliohash) e os testes (testes)
#   make teste        roda os testes de funcionamento
#   make desempenho   roda os experimentos de desempenho
#   make clean        apaga os arquivos gerados

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

all: bibliohash testes

bibliohash: main.o tabela_hash.o
	$(CC) $(CFLAGS) -o bibliohash main.o tabela_hash.o

# testes.c usa a função log() de math.h, por isso o -lm
testes: testes.o tabela_hash.o
	$(CC) $(CFLAGS) -o testes testes.o tabela_hash.o -lm

# Regra geral: cada .c vira um .o (recompila se o .h mudar)
%.o: %.c tabela_hash.h
	$(CC) $(CFLAGS) -c $<

teste: testes
	./testes funcionais

desempenho: testes
	./testes desempenho

clean:
	rm -f *.o bibliohash testes

.PHONY: all teste desempenho clean
