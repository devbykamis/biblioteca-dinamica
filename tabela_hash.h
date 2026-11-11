#ifndef TABELA_HASH_H
#define TABELA_HASH_H

#define TAM_TELEFONE 16
#define TAM_NOME 100
#define TAM_EMAIL 100
#define TAM_OBS 200

#define CAPACIDADE_INICIAL 11

#define FATOR_CARGA_MAXIMO 0.7

#define OK 1
#define ERRO_DUPLICADO 0
#define ERRO_NAO_ENCONTRADO -1
#define ERRO_TELEFONE_INVALIDO -2
#define ERRO_MEMORIA -3
#define ERRO_TABELA_CHEIA -4

typedef enum {
    VAZIO = 0,
    OCUPADO = 1,
    REMOVIDO = 2
} Estado;

typedef struct {
    char telefone[TAM_TELEFONE];
    char nome[TAM_NOME];
    char email[TAM_EMAIL];
    char observacoes[TAM_OBS];
    Estado estado;
} Contato;

typedef struct {
    Contato *vetor;
    int capacidade;
    int quantidade;
    int removidos;
    long colisoes;
    int redimensionamentos;
    double fator_maximo;
    long sondagens_busca;
} TabelaHash;

TabelaHash *criar_tabela(int capacidade_inicial);
void destruir_tabela(TabelaHash *tabela);
int proximo_primo(int n);

int normalizar_telefone(const char *entrada, char *saida);

int hash1(const char *telefone, int m);
int hash2(const char *telefone, int m);
int posicao_sondagem(int h1, int h2, int i, int m);

int inserir_contato(TabelaHash *tabela, const char *telefone, const char *nome,
                    const char *email, const char *observacoes);
Contato *buscar_contato(TabelaHash *tabela, const char *telefone);
int atualizar_contato(TabelaHash *tabela, const char *telefone, const char *nome,
                      const char *email, const char *observacoes);
int remover_contato(TabelaHash *tabela, const char *telefone);

void listar_contatos(const TabelaHash *tabela);
void exibir_estrutura(const TabelaHash *tabela);

double fator_de_carga(const TabelaHash *tabela);

#endif
