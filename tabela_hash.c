#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "tabela_hash.h"

#define POS_CHEIA -1
#define POS_DUPLICADA -2

static int eh_primo(int n) {
    if (n < 2) {
        return 0;
    }
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

int proximo_primo(int n) {
    while (!eh_primo(n)) {
        n++;
    }
    return n;
}

TabelaHash *criar_tabela(int capacidade_inicial) {
    if (capacidade_inicial < 2) {
        capacidade_inicial = CAPACIDADE_INICIAL;
    }

    TabelaHash *tabela = (TabelaHash *) malloc(sizeof(TabelaHash));
    if (tabela == NULL) {
        return NULL;
    }

    tabela->capacidade = proximo_primo(capacidade_inicial);

    tabela->vetor = (Contato *) calloc(tabela->capacidade, sizeof(Contato));
    if (tabela->vetor == NULL) {
        free(tabela);
        return NULL;
    }

    tabela->quantidade = 0;
    tabela->removidos = 0;
    tabela->colisoes = 0;
    tabela->redimensionamentos = 0;
    tabela->fator_maximo = FATOR_CARGA_MAXIMO;
    tabela->sondagens_busca = 0;

    return tabela;
}

void destruir_tabela(TabelaHash *tabela) {
    if (tabela == NULL) {
        return;
    }
    free(tabela->vetor);
    free(tabela);
}

#define MIN_DIGITOS 8
#define MAX_DIGITOS 15

int normalizar_telefone(const char *entrada, char *saida) {
    if (entrada == NULL || saida == NULL) {
        return ERRO_TELEFONE_INVALIDO;
    }

    int tamanho = 0;
    for (int i = 0; entrada[i] != '\0'; i++) {
        if (isdigit((unsigned char) entrada[i])) {
            if (tamanho >= MAX_DIGITOS) {
                saida[0] = '\0';
                return ERRO_TELEFONE_INVALIDO;
            }
            saida[tamanho] = entrada[i];
            tamanho++;
        }
    }
    saida[tamanho] = '\0';

    if (tamanho < MIN_DIGITOS) {
        return ERRO_TELEFONE_INVALIDO;
    }
    return OK;
}

static const int POSICOES_H1[] = {7, 6, 5, 4, 3, 2, 1};
#define QTD_POSICOES_H1 7

int hash1(const char *telefone, int m) {
    int tamanho = 0;
    while (telefone[tamanho] != '\0') {
        tamanho++;
    }

    long valor = 0;
    for (int i = 0; i < QTD_POSICOES_H1; i++) {
        int indice = tamanho - POSICOES_H1[i];
        if (indice >= 0) {
            valor = valor * 10 + (telefone[indice] - '0');
        }
    }
    return (int) (valor % m);
}

static const int POSICOES_H2[] = {1, 2, 3, 4, 5, 6, 7};
#define QTD_POSICOES_H2 7

int hash2(const char *telefone, int m) {
    if (m <= 2) {
        return 1;
    }

    int tamanho = 0;
    while (telefone[tamanho] != '\0') {
        tamanho++;
    }

    long valor = 0;
    for (int i = 0; i < QTD_POSICOES_H2; i++) {
        int indice = tamanho - POSICOES_H2[i];
        if (indice >= 0) {
            valor = valor * 10 + (telefone[indice] - '0');
        }
    }
    return (int) (1 + valor % (m - 2));
}

int posicao_sondagem(int h1, int h2, int i, int m) {
    return (int) (((long) h1 + (long) i * h2) % m);
}

static int precisa_redimensionar(const TabelaHash *tabela) {
    double ocupacao = tabela->quantidade + tabela->removidos + 1;
    return ocupacao > tabela->fator_maximo * tabela->capacidade;
}

static long colocar_no_vetor(Contato *vetor, int m, const Contato *contato) {
    int a = hash1(contato->telefone, m);
    int b = hash2(contato->telefone, m);
    for (int i = 0; i < m; i++) {
        int pos = posicao_sondagem(a, b, i, m);
        if (vetor[pos].estado == VAZIO) {
            vetor[pos] = *contato;
            return i;
        }
    }
    return -1;
}

static int redimensionar_tabela(TabelaHash *tabela) {
    int nova_capacidade = tabela->capacidade;
    if (tabela->quantidade + 1 > tabela->fator_maximo * tabela->capacidade / 2.0) {
        nova_capacidade = proximo_primo(2 * tabela->capacidade);
    }

    Contato *novo_vetor = (Contato *) calloc(nova_capacidade, sizeof(Contato));
    if (novo_vetor == NULL) {
        return ERRO_MEMORIA;
    }

    long colisoes_novas = 0;
    for (int i = 0; i < tabela->capacidade; i++) {
        if (tabela->vetor[i].estado == OCUPADO) {
            long puladas = colocar_no_vetor(novo_vetor, nova_capacidade, &tabela->vetor[i]);
            if (puladas < 0) {
                free(novo_vetor);
                return ERRO_MEMORIA;
            }
            colisoes_novas += puladas;
        }
    }

    free(tabela->vetor);
    tabela->vetor = novo_vetor;
    tabela->capacidade = nova_capacidade;
    tabela->removidos = 0;
    tabela->colisoes += colisoes_novas;
    tabela->redimensionamentos++;
    return OK;
}

static void copiar_texto(char *destino, const char *origem, int tamanho) {
    if (origem == NULL) {
        destino[0] = '\0';
        return;
    }
    strncpy(destino, origem, tamanho - 1);
    destino[tamanho - 1] = '\0';
}

static int procurar_destino(const TabelaHash *tabela, const char *chave, int *tentativas_extras) {
    int m = tabela->capacidade;
    int a = hash1(chave, m);
    int b = hash2(chave, m);
    int destino = POS_CHEIA;
    *tentativas_extras = 0;

    for (int i = 0; i < m; i++) {
        int pos = posicao_sondagem(a, b, i, m);
        const Contato *atual = &tabela->vetor[pos];

        if (atual->estado == OCUPADO) {
            if (strcmp(atual->telefone, chave) == 0) {
                return POS_DUPLICADA;
            }
        } else {
            if (destino == POS_CHEIA) {
                destino = pos;
                *tentativas_extras = i;
            }
            if (atual->estado == VAZIO) {
                break;
            }
        }
    }
    return destino;
}

int inserir_contato(TabelaHash *tabela, const char *telefone, const char *nome,
                    const char *email, const char *observacoes) {
    if (tabela == NULL) {
        return ERRO_MEMORIA;
    }

    char chave[TAM_TELEFONE];
    if (normalizar_telefone(telefone, chave) != OK) {
        return ERRO_TELEFONE_INVALIDO;
    }

    int tentativas_extras;
    int destino = procurar_destino(tabela, chave, &tentativas_extras);
    if (destino == POS_DUPLICADA) {
        return ERRO_DUPLICADO;
    }

    if (destino == POS_CHEIA || precisa_redimensionar(tabela)) {
        if (redimensionar_tabela(tabela) == OK) {
            destino = procurar_destino(tabela, chave, &tentativas_extras);
        }
    }
    if (destino < 0) {
        return (destino == POS_CHEIA) ? ERRO_TABELA_CHEIA : ERRO_DUPLICADO;
    }

    Contato *novo = &tabela->vetor[destino];
    if (novo->estado == REMOVIDO) {
        tabela->removidos--;
    }

    strcpy(novo->telefone, chave);
    copiar_texto(novo->nome, nome, TAM_NOME);
    copiar_texto(novo->email, email, TAM_EMAIL);
    copiar_texto(novo->observacoes, observacoes, TAM_OBS);
    novo->estado = OCUPADO;

    tabela->quantidade++;
    tabela->colisoes += tentativas_extras;
    return OK;
}

Contato *buscar_contato(TabelaHash *tabela, const char *telefone) {
    if (tabela == NULL) {
        return NULL;
    }

    char chave[TAM_TELEFONE];
    if (normalizar_telefone(telefone, chave) != OK) {
        return NULL;
    }

    int m = tabela->capacidade;
    int a = hash1(chave, m);
    int b = hash2(chave, m);

    for (int i = 0; i < m; i++) {
        int pos = posicao_sondagem(a, b, i, m);
        Contato *atual = &tabela->vetor[pos];
        tabela->sondagens_busca++;

        if (atual->estado == VAZIO) {
            return NULL;
        }
        if (atual->estado == OCUPADO && strcmp(atual->telefone, chave) == 0) {
            return atual;
        }
    }
    return NULL;
}

int atualizar_contato(TabelaHash *tabela, const char *telefone, const char *nome,
                      const char *email, const char *observacoes) {
    char chave[TAM_TELEFONE];
    if (normalizar_telefone(telefone, chave) != OK) {
        return ERRO_TELEFONE_INVALIDO;
    }

    Contato *contato = buscar_contato(tabela, chave);
    if (contato == NULL) {
        return ERRO_NAO_ENCONTRADO;
    }

    if (nome != NULL) {
        copiar_texto(contato->nome, nome, TAM_NOME);
    }
    if (email != NULL) {
        copiar_texto(contato->email, email, TAM_EMAIL);
    }
    if (observacoes != NULL) {
        copiar_texto(contato->observacoes, observacoes, TAM_OBS);
    }
    return OK;
}

int remover_contato(TabelaHash *tabela, const char *telefone) {
    char chave[TAM_TELEFONE];
    if (normalizar_telefone(telefone, chave) != OK) {
        return ERRO_TELEFONE_INVALIDO;
    }

    Contato *contato = buscar_contato(tabela, chave);
    if (contato == NULL) {
        return ERRO_NAO_ENCONTRADO;
    }

    contato->telefone[0] = '\0';
    contato->nome[0] = '\0';
    contato->email[0] = '\0';
    contato->observacoes[0] = '\0';
    contato->estado = REMOVIDO;

    tabela->quantidade--;
    tabela->removidos++;
    return OK;
}

void listar_contatos(const TabelaHash *tabela) {
    if (tabela == NULL || tabela->quantidade == 0) {
        printf("(nenhum contato cadastrado)\n");
        return;
    }

    printf("=== Contatos cadastrados: %d ===\n", tabela->quantidade);
    for (int i = 0; i < tabela->capacidade; i++) {
        const Contato *c = &tabela->vetor[i];
        if (c->estado == OCUPADO) {
            printf("Telefone: %s | Nome: %s | E-mail: %s | Obs: %s\n",
                   c->telefone, c->nome, c->email, c->observacoes);
        }
    }
}

void exibir_estrutura(const TabelaHash *tabela) {
    if (tabela == NULL) {
        printf("(tabela inexistente)\n");
        return;
    }

    printf("=== Estrutura da Tabela Hash ===\n");
    printf("Capacidade (m): %d | Contatos (n): %d | Removidos: %d\n",
           tabela->capacidade, tabela->quantidade, tabela->removidos);
    printf("Fator de carga (n/m): %.3f | Colisões: %ld | Redimensionamentos: %d\n",
           fator_de_carga(tabela), tabela->colisoes, tabela->redimensionamentos);

    for (int i = 0; i < tabela->capacidade; i++) {
        const Contato *c = &tabela->vetor[i];
        if (c->estado == VAZIO) {
            printf("[%4d] VAZIO\n", i);
        } else if (c->estado == REMOVIDO) {
            printf("[%4d] REMOVIDO (tombstone)\n", i);
        } else {
            int casa = hash1(c->telefone, tabela->capacidade);
            printf("[%4d] OCUPADO  %s (%s) h1=%d%s\n", i, c->telefone, c->nome, casa,
                   casa == i ? "" : "  <- deslocado por colisão");
        }
    }
}

double fator_de_carga(const TabelaHash *tabela) {
    if (tabela == NULL || tabela->capacidade == 0) {
        return 0.0;
    }
    return (double) tabela->quantidade / tabela->capacidade;
}
