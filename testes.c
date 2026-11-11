#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "tabela_hash.h"

static int total_verificacoes = 0;
static int total_falhas = 0;

static void verificar(int condicao, const char *descricao) {
    total_verificacoes++;
    if (condicao) {
        printf("  [OK]   %s\n", descricao);
    } else {
        printf("  [ERRO] %s\n", descricao);
        total_falhas++;
    }
}

static void gerar_colisoes(int m, int quantidade, char destino[][TAM_TELEFONE]) {
    int alvo = hash1("84991230000", m);
    int achadas = 0;
    for (int i = 0; achadas < quantidade; i++) {
        char telefone[TAM_TELEFONE];
        sprintf(telefone, "8499%07d", 1230000 + i);
        if (hash1(telefone, m) == alvo) {
            strcpy(destino[achadas], telefone);
            achadas++;
        }
    }
}

static void teste_1_insercao_e_busca(void) {
    printf("\nTeste 1: inserção e busca de chave existente\n");
    TabelaHash *t = criar_tabela(11);

    verificar(inserir_contato(t, "(84) 99123-4567", "Ana Souza", "ana@escola.br", "Aluna do 3o ano") == OK,
              "inserção devolve OK");
    Contato *c = buscar_contato(t, "84991234567");
    verificar(c != NULL, "contato é encontrado pelo telefone");
    verificar(c != NULL && strcmp(c->nome, "Ana Souza") == 0, "nome guardado corretamente");
    verificar(c != NULL && strcmp(c->email, "ana@escola.br") == 0, "e-mail guardado corretamente");
    verificar(c != NULL && strcmp(c->observacoes, "Aluna do 3o ano") == 0, "observações guardadas corretamente");
    verificar(c != NULL && strcmp(c->telefone, "84991234567") == 0, "telefone guardado normalizado");
    verificar(buscar_contato(t, "84 99123 4567") == c, "outra formatação do telefone encontra o mesmo contato");
    verificar(t->quantidade == 1, "quantidade = 1");

    destruir_tabela(t);
}

static void teste_2_busca_inexistente(void) {
    printf("\nTeste 2: busca de chave inexistente\n");
    TabelaHash *t = criar_tabela(11);
    inserir_contato(t, "84991234567", "Ana", "a@x.com", "");

    verificar(buscar_contato(t, "84999999999") == NULL, "telefone não cadastrado devolve NULL");
    verificar(buscar_contato(t, "abc") == NULL, "telefone inválido devolve NULL");
    verificar(buscar_contato(t, NULL) == NULL, "telefone NULL devolve NULL");

    destruir_tabela(t);
}

static void teste_3_chave_duplicada(void) {
    printf("\nTeste 3: inserção de chave duplicada\n");
    TabelaHash *t = criar_tabela(11);
    inserir_contato(t, "84991234567", "Ana", "ana@x.com", "original");

    verificar(inserir_contato(t, "84991234567", "Outra", "o@x.com", "x") == ERRO_DUPLICADO,
              "mesmo formato é rejeitado");
    verificar(inserir_contato(t, "+(84) 99123-4567", "Outra", "o@x.com", "x") == ERRO_DUPLICADO,
              "outro formato do mesmo número também é rejeitado");
    verificar(t->quantidade == 1, "quantidade continua 1");
    Contato *c = buscar_contato(t, "84991234567");
    verificar(c != NULL && strcmp(c->nome, "Ana") == 0 && strcmp(c->observacoes, "original") == 0,
              "os dados originais não foram alterados");

    destruir_tabela(t);
}

static void teste_4_colisoes(void) {
    printf("\nTeste 4: inserção de chaves que colidem\n");
    TabelaHash *t = criar_tabela(11);
    char chaves[4][TAM_TELEFONE];
    gerar_colisoes(t->capacidade, 4, chaves);

    int mesmo_h1 = 1;
    for (int i = 1; i < 4; i++) {
        if (hash1(chaves[i], t->capacidade) != hash1(chaves[0], t->capacidade)) {
            mesmo_h1 = 0;
        }
    }
    verificar(mesmo_h1, "as 4 chaves de teste têm o mesmo h1 (colidem na posição inicial)");

    int todas_ok = 1;
    for (int i = 0; i < 4; i++) {
        if (inserir_contato(t, chaves[i], "Colidiu", "c@x.com", "") != OK) {
            todas_ok = 0;
        }
    }
    verificar(todas_ok, "as 4 inserções funcionam");
    verificar(t->colisoes > 0, "as colisões foram contabilizadas");
    printf("         (colisões registradas: %ld)\n", t->colisoes);

    int todas_achadas = 1;
    for (int i = 0; i < 4; i++) {
        if (buscar_contato(t, chaves[i]) == NULL) {
            todas_achadas = 0;
        }
    }
    verificar(todas_achadas, "todas as chaves que colidiram são encontradas");

    int posicoes_distintas = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (buscar_contato(t, chaves[i]) == buscar_contato(t, chaves[j])) {
                posicoes_distintas = 0;
            }
        }
    }
    verificar(posicoes_distintas, "cada chave ocupa uma posição diferente");

    destruir_tabela(t);
}

static void teste_5_remocao_com_colisao(void) {
    printf("\nTeste 5: remoção de chave com colisão e busca das demais\n");
    char chaves[4][TAM_TELEFONE];

    TabelaHash *t = criar_tabela(11);
    gerar_colisoes(t->capacidade, 4, chaves);
    for (int i = 0; i < 4; i++) {
        inserir_contato(t, chaves[i], "Colidiu", "c@x.com", "");
    }
    verificar(remover_contato(t, chaves[0]) == OK, "remove a primeira chave inserida");
    verificar(buscar_contato(t, chaves[0]) == NULL, "chave removida não é mais encontrada");
    verificar(buscar_contato(t, chaves[1]) != NULL && buscar_contato(t, chaves[2]) != NULL
              && buscar_contato(t, chaves[3]) != NULL,
              "as demais continuam sendo encontradas (a busca passa pelo tombstone)");
    verificar(t->quantidade == 3 && t->removidos == 1, "quantidade = 3 e removidos = 1");
    verificar(remover_contato(t, chaves[0]) == ERRO_NAO_ENCONTRADO, "remover de novo devolve ERRO_NAO_ENCONTRADO");
    destruir_tabela(t);

    t = criar_tabela(11);
    for (int i = 0; i < 4; i++) {
        inserir_contato(t, chaves[i], "Colidiu", "c@x.com", "");
    }
    verificar(remover_contato(t, chaves[1]) == OK, "remove uma chave deslocada por colisão");
    verificar(buscar_contato(t, chaves[0]) != NULL && buscar_contato(t, chaves[2]) != NULL
              && buscar_contato(t, chaves[3]) != NULL,
              "as demais continuam sendo encontradas");

    verificar(inserir_contato(t, chaves[1], "Voltou", "v@x.com", "") == OK, "reinserção da chave removida funciona");
    verificar(t->removidos == 0, "o tombstone foi reaproveitado (removidos = 0)");
    verificar(buscar_contato(t, chaves[1]) != NULL && buscar_contato(t, chaves[3]) != NULL,
              "todas as chaves são encontradas após a reinserção");
    destruir_tabela(t);
}

static void teste_6_atualizacao(void) {
    printf("\nTeste 6: atualização sem alterar a chave\n");
    TabelaHash *t = criar_tabela(11);
    inserir_contato(t, "84991234567", "Ana", "ana@x.com", "antigo");
    Contato *antes = buscar_contato(t, "84991234567");

    verificar(atualizar_contato(t, "(84) 99123-4567", "Ana Maria", "ana.maria@x.com", "novo") == OK,
              "atualização (com outra formatação do telefone) devolve OK");
    Contato *depois = buscar_contato(t, "84991234567");
    verificar(depois == antes, "o contato continua na mesma posição");
    verificar(strcmp(depois->telefone, "84991234567") == 0, "a chave (telefone) não mudou");
    verificar(strcmp(depois->nome, "Ana Maria") == 0 && strcmp(depois->email, "ana.maria@x.com") == 0
              && strcmp(depois->observacoes, "novo") == 0, "nome, e-mail e observações foram atualizados");
    verificar(t->quantidade == 1, "quantidade continua 1");

    verificar(atualizar_contato(t, "84991234567", NULL, "outro@x.com", NULL) == OK, "campos NULL são mantidos");
    verificar(strcmp(depois->nome, "Ana Maria") == 0 && strcmp(depois->email, "outro@x.com") == 0
              && strcmp(depois->observacoes, "novo") == 0, "só o e-mail mudou");

    verificar(atualizar_contato(t, "84999999999", "X", "x", "x") == ERRO_NAO_ENCONTRADO,
              "atualizar telefone inexistente devolve ERRO_NAO_ENCONTRADO");
    verificar(atualizar_contato(t, "123", "X", "x", "x") == ERRO_TELEFONE_INVALIDO,
              "atualizar telefone inválido devolve ERRO_TELEFONE_INVALIDO");

    destruir_tabela(t);
}

static void teste_7_limites(void) {
    printf("\nTeste 7: tabela vazia e situações de limite\n");

    TabelaHash *t = criar_tabela(11);
    verificar(t->quantidade == 0 && fator_de_carga(t) == 0.0, "tabela nova: n = 0 e α = 0");
    verificar(buscar_contato(t, "84991234567") == NULL, "busca em tabela vazia devolve NULL");
    verificar(remover_contato(t, "84991234567") == ERRO_NAO_ENCONTRADO, "remoção em tabela vazia devolve ERRO_NAO_ENCONTRADO");
    verificar(atualizar_contato(t, "84991234567", "a", "b", "c") == ERRO_NAO_ENCONTRADO, "atualização em tabela vazia devolve ERRO_NAO_ENCONTRADO");
    printf("         listagem e estrutura da tabela vazia:\n         ");
    listar_contatos(t);
    exibir_estrutura(t);
    destruir_tabela(t);

    t = criar_tabela(0);
    verificar(t != NULL && t->capacidade == CAPACIDADE_INICIAL, "capacidade 0 usa a capacidade padrão");
    destruir_tabela(t);
    t = criar_tabela(-5);
    verificar(t != NULL && t->capacidade == CAPACIDADE_INICIAL, "capacidade negativa usa a capacidade padrão");
    destruir_tabela(t);
    t = criar_tabela(10);
    verificar(t->capacidade == 11, "capacidade 10 vira 11 (primo)");
    destruir_tabela(t);

    t = criar_tabela(2);
    verificar(t->capacidade == 2, "tabela com m = 2 é criada");
    char tel[TAM_TELEFONE];
    int todos_ok = 1;
    for (int i = 0; i < 30; i++) {
        sprintf(tel, "8499%07d", 1000000 + i * 17);
        if (inserir_contato(t, tel, "N", "e", "o") != OK) {
            todos_ok = 0;
        }
    }
    verificar(todos_ok && t->quantidade == 30, "30 inserções a partir de m = 2 funcionam (a tabela cresce)");
    int todos_achados = 1;
    for (int i = 0; i < 30; i++) {
        sprintf(tel, "8499%07d", 1000000 + i * 17);
        if (buscar_contato(t, tel) == NULL) {
            todos_achados = 0;
        }
    }
    verificar(todos_achados, "os 30 contatos são encontrados");
    destruir_tabela(t);

    t = criar_tabela(11);
    verificar(inserir_contato(t, "12345678", "min", "", "") == OK, "telefone com 8 dígitos (mínimo) é aceito");
    verificar(inserir_contato(t, "123456789012345", "max", "", "") == OK, "telefone com 15 dígitos (máximo) é aceito");
    verificar(inserir_contato(t, "1234567", "x", "", "") == ERRO_TELEFONE_INVALIDO, "7 dígitos é rejeitado");
    verificar(inserir_contato(t, "1234567890123456", "x", "", "") == ERRO_TELEFONE_INVALIDO, "16 dígitos é rejeitado");
    verificar(inserir_contato(t, "", "x", "", "") == ERRO_TELEFONE_INVALIDO, "telefone vazio é rejeitado");
    verificar(inserir_contato(t, NULL, "x", "", "") == ERRO_TELEFONE_INVALIDO, "telefone NULL é rejeitado");
    verificar(t->quantidade == 2, "só os 2 contatos válidos foram guardados");

    char longo[500];
    memset(longo, 'A', 499);
    longo[499] = '\0';
    verificar(inserir_contato(t, "84991111111", longo, longo, longo) == OK, "textos enormes são aceitos");
    Contato *c = buscar_contato(t, "84991111111");
    verificar(c != NULL && strlen(c->nome) == TAM_NOME - 1 && strlen(c->email) == TAM_EMAIL - 1
              && strlen(c->observacoes) == TAM_OBS - 1, "textos cortados no limite, com '\\0' final");
    verificar(inserir_contato(t, "84992222222", NULL, NULL, NULL) == OK
              && strcmp(buscar_contato(t, "84992222222")->nome, "") == 0, "textos NULL viram textos vazios");

    verificar(inserir_contato(NULL, "84991234567", "a", "b", "c") == ERRO_MEMORIA, "inserir em tabela NULL devolve erro");
    verificar(buscar_contato(NULL, "84991234567") == NULL, "buscar em tabela NULL devolve NULL");
    destruir_tabela(NULL);
    verificar(1, "destruir_tabela(NULL) não quebra");
    destruir_tabela(t);

    t = criar_tabela(11);
    for (int i = 0; i < 1000; i++) {
        sprintf(tel, "8499%07d", 2000000 + i * 31);
        inserir_contato(t, tel, "N", "e", "o");
        remover_contato(t, tel);
    }
    verificar(t->quantidade == 0, "após 1000 ciclos de inserir/remover a tabela está vazia");
    verificar(t->capacidade == 11, "a capacidade não cresceu (tombstones são limpos pelo rehashing)");
    verificar(t->redimensionamentos > 0, "houve rehashing para limpar tombstones");
    verificar(inserir_contato(t, "84993333333", "Novo", "n@x.com", "") == OK
              && buscar_contato(t, "84993333333") != NULL, "a tabela continua funcionando");
    destruir_tabela(t);
}

static void teste_8_redimensionamento(void) {
    printf("\nTeste 8: redimensionamento e preservação dos registros\n");
    TabelaHash *t = criar_tabela(11);
    int capacidade_antes = t->capacidade;
    char tel[TAM_TELEFONE], nome[TAM_NOME];
    int total = 500;

    int todos_ok = 1;
    for (int i = 0; i < total; i++) {
        sprintf(tel, "8499%07d", 3000000 + i * 13);
        sprintf(nome, "Contato %d", i);
        if (inserir_contato(t, tel, nome, "c@x.com", "obs") != OK) {
            todos_ok = 0;
        }
    }
    verificar(todos_ok, "500 inserções a partir de m = 11 funcionam");
    verificar(t->redimensionamentos > 0, "a tabela foi redimensionada");
    verificar(t->capacidade > capacidade_antes, "a capacidade aumentou");
    printf("         (capacidade 11 -> %d, redimensionamentos: %d)\n", t->capacidade, t->redimensionamentos);
    verificar(proximo_primo(t->capacidade) == t->capacidade, "a nova capacidade é um número primo");
    verificar(t->quantidade == total, "quantidade = 500 (nenhum registro perdido)");
    verificar(t->removidos == 0, "não há tombstones após o rehashing");
    verificar(fator_de_carga(t) <= FATOR_CARGA_MAXIMO, "o fator de carga ficou dentro do limite");

    int dados_ok = 1;
    for (int i = 0; i < total; i++) {
        sprintf(tel, "8499%07d", 3000000 + i * 13);
        sprintf(nome, "Contato %d", i);
        Contato *c = buscar_contato(t, tel);
        if (c == NULL || strcmp(c->nome, nome) != 0 || strcmp(c->email, "c@x.com") != 0
            || strcmp(c->observacoes, "obs") != 0) {
            dados_ok = 0;
        }
    }
    verificar(dados_ok, "os 500 registros foram preservados com todos os dados");

    sprintf(tel, "8499%07d", 3000000);
    verificar(inserir_contato(t, tel, "Dup", "", "") == ERRO_DUPLICADO, "duplicado ainda é detectado após o rehashing");

    for (int i = 0; i < total; i += 2) {
        sprintf(tel, "8499%07d", 3000000 + i * 13);
        remover_contato(t, tel);
    }
    int restantes_ok = 1;
    for (int i = 1; i < total; i += 2) {
        sprintf(tel, "8499%07d", 3000000 + i * 13);
        if (buscar_contato(t, tel) == NULL) {
            restantes_ok = 0;
        }
    }
    verificar(restantes_ok && t->quantidade == total / 2, "após remover metade, a outra metade continua encontrada");
    for (int i = 0; i < 2000; i++) {
        sprintf(tel, "8488%07d", 5000000 + i * 7);
        inserir_contato(t, tel, "Extra", "", "");
    }
    restantes_ok = 1;
    for (int i = 1; i < total; i += 2) {
        sprintf(tel, "8499%07d", 3000000 + i * 13);
        if (buscar_contato(t, tel) == NULL) {
            restantes_ok = 0;
        }
    }
    verificar(restantes_ok && t->quantidade == total / 2 + 2000, "rehashing com tombstones preserva todos os registros");
    exibir_estrutura(NULL);
    verificar(1, "exibir_estrutura(NULL) não quebra");

    destruir_tabela(t);
}

static void executar_testes(void) {
    printf("==================== TESTES DE FUNCIONAMENTO ====================\n");
    teste_1_insercao_e_busca();
    teste_2_busca_inexistente();
    teste_3_chave_duplicada();
    teste_4_colisoes();
    teste_5_remocao_com_colisao();
    teste_6_atualizacao();
    teste_7_limites();
    teste_8_redimensionamento();
    printf("\nResumo: %d verificações, %d falhas.\n", total_verificacoes, total_falhas);
    printf(total_falhas == 0 ? "TODOS OS TESTES PASSARAM.\n" : "HÁ TESTES FALHANDO.\n");
}

#define REPETICOES 5
#define MAX_CHAVES_DISTINTAS 10000000

typedef struct {
    int registros;
    int capacidade;
    double alfa;
    long colisoes;
    int redimensionamentos;
    double ns_insercao;
    double ns_busca_ok;
    double ns_busca_falha;
    double sond_ok;
    double sond_falha;
    int erros;
} Resultado;

static double agora_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

static void gerar_chaves(int n, unsigned semente, char presentes[][TAM_TELEFONE], char ausentes[][TAM_TELEFONE]) {
    srand(semente);
    unsigned char *usado = (unsigned char *) calloc(MAX_CHAVES_DISTINTAS, 1);
    for (int i = 0; i < n; i++) {
        int x;
        do {
            x = rand() % MAX_CHAVES_DISTINTAS;
        } while (usado[x]);
        usado[x] = 1;
        sprintf(presentes[i], "8499%07d", x);
        sprintf(ausentes[i], "8488%07d", rand() % MAX_CHAVES_DISTINTAS);
    }
    free(usado);
}

static void executar_cenario(int n, unsigned semente, int capacidade_inicial, double fator_maximo, Resultado *r) {
    char (*presentes)[TAM_TELEFONE] = malloc((size_t) n * TAM_TELEFONE);
    char (*ausentes)[TAM_TELEFONE] = malloc((size_t) n * TAM_TELEFONE);
    gerar_chaves(n, semente, presentes, ausentes);

    memset(r, 0, sizeof(Resultado));
    TabelaHash *t = criar_tabela(capacidade_inicial);
    t->fator_maximo = fator_maximo;

    double inicio = agora_ns();
    for (int i = 0; i < n; i++) {
        if (inserir_contato(t, presentes[i], "Nome Teste", "teste@exemplo.com", "observacao") != OK) {
            r->erros++;
        }
    }
    r->ns_insercao = (agora_ns() - inicio) / n;

    r->registros = t->quantidade;
    r->capacidade = t->capacidade;
    r->alfa = fator_de_carga(t);
    r->colisoes = t->colisoes;
    r->redimensionamentos = t->redimensionamentos;

    int passadas = 100000 / n;
    if (passadas < 1) {
        passadas = 1;
    }
    double operacoes = (double) passadas * n;

    t->sondagens_busca = 0;
    inicio = agora_ns();
    for (int p = 0; p < passadas; p++) {
        for (int i = 0; i < n; i++) {
            if (buscar_contato(t, presentes[i]) == NULL) {
                r->erros++;
            }
        }
    }
    r->ns_busca_ok = (agora_ns() - inicio) / operacoes;
    r->sond_ok = t->sondagens_busca / operacoes;

    t->sondagens_busca = 0;
    inicio = agora_ns();
    for (int p = 0; p < passadas; p++) {
        for (int i = 0; i < n; i++) {
            if (buscar_contato(t, ausentes[i]) != NULL) {
                r->erros++;
            }
        }
    }
    r->ns_busca_falha = (agora_ns() - inicio) / operacoes;
    r->sond_falha = t->sondagens_busca / operacoes;

    destruir_tabela(t);
    free(presentes);
    free(ausentes);
}

static void experimento_a(void) {
    int cenarios[3] = {100, 1000, 10000};

    printf("\n### Experimento A: crescimento automático (capacidade inicial %d, limite de carga %.2f)\n\n",
           CAPACIDADE_INICIAL, FATOR_CARGA_MAXIMO);
    printf("Resultados de cada repetição:\n\n");
    printf("| Registros | Rep. | Capacidade | α = n/m | Colisões | Redim. | Inserção (ns) | Busca OK (ns) | Busca falha (ns) | Sond. OK | Sond. falha |\n");
    printf("|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|\n");

    Resultado medias[3];
    for (int c = 0; c < 3; c++) {
        Resultado soma;
        memset(&soma, 0, sizeof(soma));
        for (int rep = 0; rep < REPETICOES; rep++) {
            Resultado r;
            executar_cenario(cenarios[c], 1000 + rep, CAPACIDADE_INICIAL, FATOR_CARGA_MAXIMO, &r);
            printf("| %d | %d | %d | %.3f | %ld | %d | %.0f | %.0f | %.0f | %.2f | %.2f |\n",
                   r.registros, rep + 1, r.capacidade, r.alfa, r.colisoes, r.redimensionamentos,
                   r.ns_insercao, r.ns_busca_ok, r.ns_busca_falha, r.sond_ok, r.sond_falha);
            soma.registros = r.registros;
            soma.capacidade += r.capacidade;
            soma.alfa += r.alfa;
            soma.colisoes += r.colisoes;
            soma.redimensionamentos += r.redimensionamentos;
            soma.ns_insercao += r.ns_insercao;
            soma.ns_busca_ok += r.ns_busca_ok;
            soma.ns_busca_falha += r.ns_busca_falha;
            soma.sond_ok += r.sond_ok;
            soma.sond_falha += r.sond_falha;
            soma.erros += r.erros;
        }
        medias[c] = soma;
        medias[c].capacidade = soma.capacidade / REPETICOES;
        medias[c].alfa = soma.alfa / REPETICOES;
        medias[c].colisoes = soma.colisoes / REPETICOES;
        medias[c].redimensionamentos = soma.redimensionamentos / REPETICOES;
        medias[c].ns_insercao = soma.ns_insercao / REPETICOES;
        medias[c].ns_busca_ok = soma.ns_busca_ok / REPETICOES;
        medias[c].ns_busca_falha = soma.ns_busca_falha / REPETICOES;
        medias[c].sond_ok = soma.sond_ok / REPETICOES;
        medias[c].sond_falha = soma.sond_falha / REPETICOES;
    }

    printf("\nMédias das %d repetições:\n\n", REPETICOES);
    printf("| Registros | Capacidade | α = n/m | Colisões | Colisões por inserção | Redim. | Inserção (ns) | Busca OK (ns) | Busca falha (ns) | Sond. OK | Sond. falha |\n");
    printf("|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|\n");
    for (int c = 0; c < 3; c++) {
        printf("| %d | %d | %.3f | %ld | %.2f | %d | %.0f | %.0f | %.0f | %.2f | %.2f |\n",
               medias[c].registros, medias[c].capacidade, medias[c].alfa, medias[c].colisoes,
               (double) medias[c].colisoes / medias[c].registros, medias[c].redimensionamentos,
               medias[c].ns_insercao, medias[c].ns_busca_ok, medias[c].ns_busca_falha,
               medias[c].sond_ok, medias[c].sond_falha);
    }
    int erros = 0;
    for (int c = 0; c < 3; c++) {
        erros += medias[c].erros;
    }
    printf("\nInconsistências detectadas (deve ser 0): %d\n", erros);
}

static void experimento_b(void) {
    double alvos[6] = {0.10, 0.30, 0.50, 0.70, 0.90, 0.95};
    int n = 10000;

    printf("\n### Experimento B: efeito do fator de carga (n = %d, capacidade fixa, sem redimensionar)\n\n", n);
    printf("Médias de %d repetições:\n\n", REPETICOES);
    printf("| α alvo | Capacidade | α real | Colisões por inserção | Inserção (ns) | Busca OK (ns) | Busca falha (ns) | Sond. OK | Teoria OK | Sond. falha | Teoria falha |\n");
    printf("|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|\n");

    int erros = 0;
    for (int k = 0; k < 6; k++) {
        int capacidade = proximo_primo((int) (n / alvos[k]));
        double alfa = 0, col = 0, ins = 0, bok = 0, bfa = 0, sok = 0, sfa = 0;
        for (int rep = 0; rep < REPETICOES; rep++) {
            Resultado r;
            executar_cenario(n, 2000 + rep, capacidade, 1.0, &r);
            alfa += r.alfa;
            col += (double) r.colisoes / r.registros;
            ins += r.ns_insercao;
            bok += r.ns_busca_ok;
            bfa += r.ns_busca_falha;
            sok += r.sond_ok;
            sfa += r.sond_falha;
            erros += r.erros + r.redimensionamentos;
        }
        alfa /= REPETICOES;
        double teoria_ok = (1.0 / alfa) * log(1.0 / (1.0 - alfa));
        double teoria_falha = 1.0 / (1.0 - alfa);
        printf("| %.2f | %d | %.3f | %.2f | %.0f | %.0f | %.0f | %.2f | %.2f | %.2f | %.2f |\n",
               alvos[k], capacidade, alfa, col / REPETICOES, ins / REPETICOES, bok / REPETICOES,
               bfa / REPETICOES, sok / REPETICOES, teoria_ok, sfa / REPETICOES, teoria_falha);
    }
    printf("\nInconsistências ou redimensionamentos inesperados (deve ser 0): %d\n", erros);
}

static void executar_desempenho(void) {
    printf("\n==================== EXPERIMENTOS DE DESEMPENHO ====================\n");
    printf("Cada cenário é repetido %d vezes com chaves aleatórias (sementes fixas).\n", REPETICOES);
    printf("Os tempos incluem a normalização do telefone e dependem da máquina.\n");
    experimento_a();
    experimento_b();
}

int main(int argc, char *argv[]) {
    int funcionais = 1, desempenho = 1;
    if (argc > 1 && strcmp(argv[1], "funcionais") == 0) {
        desempenho = 0;
    } else if (argc > 1 && strcmp(argv[1], "desempenho") == 0) {
        funcionais = 0;
    }

    if (funcionais) {
        executar_testes();
    }
    if (desempenho) {
        executar_desempenho();
    }
    return total_falhas == 0 ? 0 : 1;
}
