#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela_hash.h"

#define TAM_LINHA 256

static int ler_texto(const char *pergunta, char *destino, int tamanho) {
    printf("%s", pergunta);
    if (fgets(destino, tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return 0;
    }
    destino[strcspn(destino, "\n")] = '\0';
    return 1;
}

static void mostrar_resultado(int codigo) {
    switch (codigo) {
        case OK:
            printf("Operação realizada com sucesso.\n");
            break;
        case ERRO_DUPLICADO:
            printf("Erro: já existe um contato com esse telefone.\n");
            break;
        case ERRO_NAO_ENCONTRADO:
            printf("Contato não encontrado.\n");
            break;
        case ERRO_TELEFONE_INVALIDO:
            printf("Erro: telefone inválido (informe de 8 a 15 dígitos).\n");
            break;
        case ERRO_TABELA_CHEIA:
            printf("Erro: a tabela está cheia.\n");
            break;
        default:
            printf("Erro: falta de memória.\n");
    }
}

static void opcao_inserir(TabelaHash *tabela) {
    char telefone[TAM_LINHA], nome[TAM_LINHA], email[TAM_LINHA], obs[TAM_LINHA];
    ler_texto("Telefone: ", telefone, TAM_LINHA);
    ler_texto("Nome: ", nome, TAM_LINHA);
    ler_texto("E-mail: ", email, TAM_LINHA);
    ler_texto("Observações: ", obs, TAM_LINHA);
    mostrar_resultado(inserir_contato(tabela, telefone, nome, email, obs));
}

static void opcao_buscar(TabelaHash *tabela) {
    char telefone[TAM_LINHA];
    ler_texto("Telefone: ", telefone, TAM_LINHA);
    Contato *c = buscar_contato(tabela, telefone);
    if (c == NULL) {
        printf("Contato não encontrado.\n");
    } else {
        printf("Telefone: %s\nNome: %s\nE-mail: %s\nObservações: %s\n",
               c->telefone, c->nome, c->email, c->observacoes);
    }
}

static void opcao_atualizar(TabelaHash *tabela) {
    char telefone[TAM_LINHA], nome[TAM_LINHA], email[TAM_LINHA], obs[TAM_LINHA];
    ler_texto("Telefone do contato a atualizar: ", telefone, TAM_LINHA);
    if (buscar_contato(tabela, telefone) == NULL) {
        printf("Contato não encontrado.\n");
        return;
    }
    printf("(o telefone não pode ser alterado)\n");
    ler_texto("Novo nome: ", nome, TAM_LINHA);
    ler_texto("Novo e-mail: ", email, TAM_LINHA);
    ler_texto("Novas observações: ", obs, TAM_LINHA);
    mostrar_resultado(atualizar_contato(tabela, telefone, nome, email, obs));
}

static void opcao_remover(TabelaHash *tabela) {
    char telefone[TAM_LINHA];
    ler_texto("Telefone: ", telefone, TAM_LINHA);
    mostrar_resultado(remover_contato(tabela, telefone));
}

static void opcao_estatisticas(const TabelaHash *tabela) {
    printf("Contatos (n): %d\n", tabela->quantidade);
    printf("Capacidade (m): %d\n", tabela->capacidade);
    printf("Fator de carga (n/m): %.3f\n", fator_de_carga(tabela));
    printf("Posições removidas (tombstones): %d\n", tabela->removidos);
    printf("Colisões (posições puladas): %ld\n", tabela->colisoes);
    printf("Redimensionamentos: %d\n", tabela->redimensionamentos);
}

static void opcao_exemplos(TabelaHash *tabela) {
    const char *dados[10][4] = {
        {"(84) 99123-4567", "Ana Souza", "ana@escola.br", "Aluna do 3o ano"},
        {"84 98877-1020", "Bruno Lima", "bruno@univ.br", "Professor de Física"},
        {"84 3351-2244", "Carla Dias", "carla@biblio.org", "Bibliotecária"},
        {"(84) 99911-3030", "Diego Alves", "diego@mail.com", "Atrasou 2 livros"},
        {"84 98765-4321", "Elisa Melo", "elisa@mail.com", "Visitante frequente"},
        {"(84) 99654-7788", "Fábio Rocha", "fabio@escola.br", "Aluno do 1o ano"},
        {"84 3351-9090", "Gisele Nunes", "gisele@univ.br", "Pesquisadora"},
        {"(84) 98123-0001", "Hugo Pires", "hugo@mail.com", "Sem observações"},
        {"84 99444-5566", "Iara Costa", "iara@escola.br", "Monitora"},
        {"(84) 98222-3344", "João Braga", "joao@mail.com", "Empréstimo especial"}
    };
    int novos = 0;
    for (int i = 0; i < 10; i++) {
        if (inserir_contato(tabela, dados[i][0], dados[i][1], dados[i][2], dados[i][3]) == OK) {
            novos++;
        }
    }
    printf("%d contatos de exemplo cadastrados.\n", novos);
}

static void mostrar_menu(void) {
    printf("\n===== BiblioHash - Contatos da Biblioteca =====\n");
    printf("1 - Inserir contato\n");
    printf("2 - Buscar contato pelo telefone\n");
    printf("3 - Atualizar contato\n");
    printf("4 - Remover contato\n");
    printf("5 - Listar contatos\n");
    printf("6 - Exibir estrutura interna da tabela\n");
    printf("7 - Estatísticas (n, m, fator de carga, colisões)\n");
    printf("8 - Carregar 10 contatos de exemplo\n");
    printf("0 - Sair\n");
}

int main(void) {
    TabelaHash *tabela = criar_tabela(CAPACIDADE_INICIAL);
    if (tabela == NULL) {
        printf("Erro: não foi possível criar a tabela.\n");
        return 1;
    }

    char linha[TAM_LINHA];
    int opcao = -1;
    while (opcao != 0) {
        mostrar_menu();
        if (!ler_texto("Opção: ", linha, TAM_LINHA)) {
            break;
        }
        if (sscanf(linha, "%d", &opcao) != 1) {
            opcao = -1;
        }

        switch (opcao) {
            case 1: opcao_inserir(tabela); break;
            case 2: opcao_buscar(tabela); break;
            case 3: opcao_atualizar(tabela); break;
            case 4: opcao_remover(tabela); break;
            case 5: listar_contatos(tabela); break;
            case 6: exibir_estrutura(tabela); break;
            case 7: opcao_estatisticas(tabela); break;
            case 8: opcao_exemplos(tabela); break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opção inválida.\n");
        }
    }

    destruir_tabela(tabela);
    return 0;
}
