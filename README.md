# BiblioHash

Biblioteca reutilizável em C, baseada em **Tabela Hash**, para gerenciar os contatos dos usuários de uma biblioteca (escolar, universitária ou pública). Projeto acadêmico.

O código procura ser simples e direto (C padrão, nomes em português, comentários `//`), sem usar nenhuma estrutura pronta de outras bibliotecas: a tabela, o vetor dinâmico, as funções hash e o redimensionamento foram implementados do zero.

## 1. Configuração do trabalho

| Item | Escolha | Como foi implementado |
|---|---|---|
| **E1.2** | Endereçamento aberto com vetor dinâmico | O vetor é alocado com `calloc`. Quando a carga passa do limite, a tabela cresce e **todos** os contatos passam por rehashing. |
| **H5** | Hash por extração | `hash1` extrai posições predefinidas dos dígitos do telefone (ver seção 4). |
| **C4** | Duplo hashing | `p(i) = (h1(k) + i * h2(k)) % m`, com `h2` sempre entre 1 e m-1 e `m` sempre primo. |
| **A3** | Agenda de contatos | Chave = telefone normalizado; dados associados = nome, e-mail e observações. |

## 2. Arquivos

```text
BiblioHash/
├── tabela_hash.h   declarações: constantes, estruturas Contato e TabelaHash, funções públicas
├── tabela_hash.c   implementação da Tabela Hash
├── main.c          menu para demonstrar a biblioteca
├── testes.c        testes de funcionamento e experimentos de desempenho
├── Makefile        compilação
└── README.md       esta documentação
```

## 3. Como compilar e executar (Linux Mint, GCC)

Instale o compilador e o `make` (uma vez só):

```bash
sudo apt update
sudo apt install build-essential
```

Dentro da pasta `BiblioHash`:

```bash
make                 # compila tudo (gera ./bibliohash e ./testes)
./bibliohash         # abre o menu de demonstração
make teste           # roda os testes de funcionamento
make desempenho      # roda os experimentos de desempenho
make clean           # apaga os arquivos gerados
```

Sem o `make`, os comandos equivalentes são:

```bash
gcc -Wall -Wextra -std=c99 -g -o bibliohash main.c tabela_hash.c
gcc -Wall -Wextra -std=c99 -g -o testes testes.c tabela_hash.c -lm
./testes funcionais     # só os testes
./testes desempenho     # só os experimentos
./testes                # os dois
```

O `-lm` é necessário porque `testes.c` usa `log()` para calcular os valores teóricos do experimento B.

## 4. Decisões técnicas

**Normalização do telefone.** `normalizar_telefone` mantém só os dígitos, então `(84) 99123-4567`, `84 99123 4567` e `84991234567` viram a mesma chave `84991234567`. São aceitos de 8 a 15 dígitos; fora disso o telefone é inválido. O código do país (`+55`) **não** é removido: ele conta como dígitos do número.

**Capacidade sempre prima.** Para o duplo hashing percorrer todas as posições, o passo `h2` precisa ser primo com `m`. Se `m` é primo e `1 <= h2 <= m-1`, isso vale sempre. Por isso `criar_tabela` e o redimensionamento usam o próximo número primo (`proximo_primo`).

**H5 — posições extraídas (`hash1`).** Os dígitos são contados a partir do **final** do telefone (posição 1 = último dígito). São extraídas as posições **7, 6, 5, 4, 3, 2 e 1**, isto é, os 7 últimos dígitos, que formam um número, e o índice é esse número `% m`. Exemplo: `84991234567` extrai `1234567`.
- *Por que contar do final?* O telefone pode ter ou não DDD, mas o final do número tem sempre o mesmo significado.
- *Por que estas posições?* O início (país, DDD e o prefixo 9) é igual para quase todos os contatos de uma cidade e não ajuda a espalhar as chaves.

**C4 — segundo hash (`hash2`).** Usa os mesmos 7 dígitos, mas lidos em ordem **inversa** (`84991234567` dá `7654321`), e calcula `1 + valor % (m-2)`. O resultado fica entre 1 e m-2, nunca é zero e é primo com `m`.
- *Por que `m-2` e não `m-1`?* Durante o desenvolvimento, um teste mostrou que com `% (m-1)` todas as chaves que caíam na mesma posição inicial ficavam com o **mesmo passo**. Como `m` é primo ímpar, `m-1` é par, e o resto só enxergava os últimos dígitos do valor, quase iguais entre chaves parecidas. Com `m-2` (ímpar), os 99 passos possíveis para m = 101 apareceram entre as 198 chaves testadas.
- Os testes confirmam que, para vários valores de `m`, `m` tentativas visitam as `m` posições.

**Tombstones.** `remover_contato` marca a posição como `REMOVIDO` em vez de `VAZIO`. A busca pula tombstones e só para em uma posição `VAZIO`; a inserção guarda o primeiro tombstone como destino, mas continua procurando até uma posição vazia para garantir que não haja duplicado, e depois reaproveita o tombstone.

**Redimensionamento e rehashing.** Antes de inserir, se `(n + removidos + 1) / m > 0,7` (`FATOR_CARGA_MAXIMO`), a tabela é reconstruída. Os tombstones entram na conta porque também alongam as sondagens.
- Se há muitos contatos de verdade, a capacidade dobra (próximo primo >= 2m).
- Se a ocupação era formada principalmente por tombstones, o tamanho é mantido e a tabela é apenas refeita, o que elimina os tombstones. Isso impede que ciclos de inserir/remover façam a tabela crescer sem parar (há um teste para isso).
- Como o índice depende de `m`, cada contato é colocado de novo com `hash1`/`hash2`. Se faltar memória, a tabela antiga continua intacta.

**Contadores.**
- `colisoes`: total de posições puladas ao colocar contatos, somando as inserções e as recolocações feitas no rehashing.
- `sondagens_busca`: total de posições examinadas nas buscas (usado nos experimentos).
- `redimensionamentos`: quantas vezes a tabela foi reconstruída.
- Fator de carga: `fator_de_carga(tabela)` devolve α = n/m.

**Textos.** Nome, e-mail e observações têm tamanho máximo (`TAM_NOME` 100, `TAM_EMAIL` 100, `TAM_OBS` 200, incluindo o `'\0'`). Textos maiores são cortados, sem estourar a memória. Cada `Contato` ocupa 420 bytes (`sizeof(Contato)`).

## 5. Funções da biblioteca

| Função | Finalidade |
|---|---|
| `criar_tabela(capacidade)` | Cria a tabela (capacidade vira o próximo primo). Devolve `NULL` se faltar memória. |
| `destruir_tabela(t)` | Libera o vetor e a estrutura. Aceita `NULL`. |
| `proximo_primo(n)` | Menor primo maior ou igual a `n`. |
| `normalizar_telefone(entrada, saida)` | Deixa só os dígitos; valida o tamanho. |
| `hash1(tel, m)` | Hash por extração (H5): posição inicial. |
| `hash2(tel, m)` | Passo do duplo hashing (C4), entre 1 e m-1. |
| `posicao_sondagem(h1, h2, i, m)` | Fórmula `p(i) = (h1 + i*h2) % m`. |
| `inserir_contato(t, tel, nome, email, obs)` | Insere; recusa telefones duplicados ou inválidos; redimensiona se preciso. |
| `buscar_contato(t, tel)` | Devolve o contato ou `NULL` se não existir. |
| `atualizar_contato(t, tel, nome, email, obs)` | Altera nome/e-mail/observações sem mudar a chave. Campo `NULL` = mantém o valor. |
| `remover_contato(t, tel)` | Remove deixando um tombstone. |
| `listar_contatos(t)` | Mostra os contatos cadastrados (na ordem do vetor). |
| `exibir_estrutura(t)` | Mostra resumo e cada posição (`VAZIO`, `OCUPADO`, `REMOVIDO`), indicando contatos deslocados por colisão. |
| `fator_de_carga(t)` | Devolve α = n/m. |

Códigos de retorno: `OK`, `ERRO_DUPLICADO`, `ERRO_NAO_ENCONTRADO`, `ERRO_TELEFONE_INVALIDO`, `ERRO_MEMORIA`, `ERRO_TABELA_CHEIA`.

Exemplo de uso:

```c
TabelaHash *t = criar_tabela(CAPACIDADE_INICIAL);
inserir_contato(t, "(84) 99123-4567", "Ana Souza", "ana@escola.br", "Aluna");
Contato *c = buscar_contato(t, "84 99123 4567");
if (c == NULL) printf("Contato não encontrado.\n");
destruir_tabela(t);
```

## 6. Testes

`make teste` executa 78 verificações, organizadas nos 8 testes pedidos:

1. Inserção e busca de chave existente (inclusive com outra formatação do telefone).
2. Busca de chave inexistente (e de telefone inválido ou `NULL`).
3. Inserção de chave duplicada (mesmo formato e formatos diferentes do mesmo número).
4. Chaves que colidem (4 telefones com o mesmo `hash1`), com contagem de colisões.
5. Remoção de chave que sofreu colisão: remove a primeira e depois uma do meio da sequência, e confirma que as demais continuam sendo encontradas; também testa o reaproveitamento do tombstone.
6. Atualização sem alterar a chave (mesma posição, mesmo telefone; campos `NULL` mantidos; erros para telefone inexistente ou inválido).
7. Tabela vazia e limites: operações em tabela vazia, capacidades inválidas, tabela mínima (m = 2), telefones com 8 e 15 dígitos (e 7 e 16), textos enormes, ponteiros `NULL` e 1000 ciclos de inserir/remover.
8. Redimensionamento e preservação: 500 inserções a partir de m = 11, conferindo capacidade prima, dados intactos, ausência de tombstones, duplicados ainda detectados e rehashing com tombstones presentes.

Resultado da execução: **78 verificações, 0 falhas**. Os testes também foram executados com `-fsanitize=address,undefined` (AddressSanitizer e UndefinedBehaviorSanitizer), sem nenhum erro de memória ou vazamento reportado. O menu (`main.c`) foi exercitado com entradas simuladas sob o mesmo sanitizer, também sem problemas.

## 7. Experimentos de desempenho

**Ambiente da execução registrada abaixo:** Ubuntu 24.04, GCC 13.3.0, x86_64, 1 CPU, compilação com `-Wall -Wextra -std=c99 -g` (sem otimização, igual ao `Makefile`). Os valores abaixo vêm da execução real de `./testes desempenho`; **os tempos dependem da máquina** e variam de uma execução para outra, então rode `make desempenho` no seu computador e compare. Já as colisões e sondagens tendem a se repetir, porque as chaves são geradas com sementes fixas (com o `rand()` da glibc, o mesmo do Linux Mint).

**Método.**
- Cada cenário é repetido **5 vezes** com telefones aleatórios diferentes (`"8499"` + 7 dígitos sem repetição para os contatos; `"8488"` + 7 dígitos para as buscas malsucedidas, que por construção nunca existem na tabela).
- *Tempo de inserção:* tempo total de inserir os `n` contatos dividido por `n`; inclui o rehashing, então é um custo médio amortizado.
- *Tempo de busca:* as buscas são repetidas em várias passadas (100000/n) para o tempo ficar mensurável; o tempo inclui a normalização do telefone.
- *Sondagens:* média de posições examinadas por busca.
- Durante os experimentos, o programa verifica se toda busca bem-sucedida acha o contato e se nenhuma malsucedida acha algo: **0 inconsistências** nos dois experimentos.

### Experimento A: a tabela começa com capacidade 11 e cresce sozinha

Mostra o vetor dinâmico e o rehashing em ação, com 100, 1.000 e 10.000 registros.

Resultados de cada repetição:

| Registros | Rep. | Capacidade | α = n/m | Colisões | Redim. | Inserção (ns) | Busca OK (ns) | Busca falha (ns) | Sond. OK | Sond. falha |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 100 | 1 | 197 | 0.508 | 105 | 4 | 929 | 134 | 128 | 1.32 | 2.07 |
| 100 | 2 | 197 | 0.508 | 86 | 4 | 360 | 134 | 128 | 1.36 | 1.88 |
| 100 | 3 | 197 | 0.508 | 104 | 4 | 462 | 136 | 130 | 1.40 | 1.93 |
| 100 | 4 | 197 | 0.508 | 110 | 4 | 467 | 133 | 131 | 1.43 | 2.06 |
| 100 | 5 | 197 | 0.508 | 104 | 4 | 444 | 130 | 120 | 1.46 | 1.63 |
| 1000 | 1 | 1597 | 0.626 | 1266 | 7 | 410 | 134 | 136 | 1.54 | 2.74 |
| 1000 | 2 | 1597 | 0.626 | 1225 | 7 | 377 | 138 | 139 | 1.52 | 2.68 |
| 1000 | 3 | 1597 | 0.626 | 1294 | 7 | 387 | 137 | 144 | 1.52 | 2.74 |
| 1000 | 4 | 1597 | 0.626 | 1376 | 7 | 572 | 213 | 191 | 1.58 | 2.58 |
| 1000 | 5 | 1597 | 0.626 | 1341 | 7 | 567 | 156 | 180 | 1.55 | 2.55 |
| 10000 | 1 | 25717 | 0.389 | 15466 | 11 | 1881 | 159 | 213 | 1.27 | 1.64 |
| 10000 | 2 | 25717 | 0.389 | 15545 | 11 | 1417 | 173 | 150 | 1.26 | 1.65 |
| 10000 | 3 | 25717 | 0.389 | 15779 | 11 | 1389 | 152 | 151 | 1.27 | 1.64 |
| 10000 | 4 | 25717 | 0.389 | 15459 | 11 | 1390 | 147 | 152 | 1.28 | 1.63 |
| 10000 | 5 | 25717 | 0.389 | 15533 | 11 | 1357 | 162 | 153 | 1.27 | 1.65 |

Médias das 5 repetições (capacidade, colisões e redimensionamentos arredondados para baixo):

| Registros | Capacidade | α = n/m | Colisões | Colisões por inserção | Redim. | Inserção (ns) | Busca OK (ns) | Busca falha (ns) | Sond. OK | Sond. falha |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 100 | 197 | 0.508 | 101 | 1.01 | 4 | 533 | 133 | 127 | 1.39 | 1.91 |
| 1000 | 1597 | 0.626 | 1300 | 1.30 | 7 | 463 | 155 | 158 | 1.54 | 2.66 |
| 10000 | 25717 | 0.389 | 15556 | 1.56 | 11 | 1487 | 159 | 164 | 1.27 | 1.64 |

### Experimento B: capacidade fixa, para isolar o efeito do fator de carga

Aqui o redimensionamento é desligado (limite de carga = 1,0) e a capacidade é escolhida para atingir cada α, com n = 10.000 registros. As colunas "Teoria" são calculadas pelo programa com as fórmulas clássicas de hashing uniforme (busca bem-sucedida: `(1/α)·ln(1/(1−α))`; malsucedida: `1/(1−α)`); não são medições.

| α alvo | Capacidade | α real | Colisões por inserção | Inserção (ns) | Busca OK (ns) | Busca falha (ns) | Sond. OK | Teoria OK | Sond. falha | Teoria falha |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 0.10 | 100003 | 0.100 | 0.05 | 2150 | 177 | 154 | 1.05 | 1.05 | 1.11 | 1.11 |
| 0.30 | 33343 | 0.300 | 0.19 | 538 | 157 | 141 | 1.19 | 1.19 | 1.42 | 1.43 |
| 0.50 | 20011 | 0.500 | 0.39 | 247 | 155 | 155 | 1.39 | 1.39 | 2.00 | 2.00 |
| 0.70 | 14293 | 0.700 | 0.72 | 231 | 148 | 153 | 1.72 | 1.72 | 3.33 | 3.33 |
| 0.90 | 11113 | 0.900 | 1.54 | 229 | 152 | 203 | 2.54 | 2.56 | 10.04 | 9.98 |
| 0.95 | 10529 | 0.950 | 2.17 | 232 | 159 | 277 | 3.17 | 3.15 | 19.99 | 19.90 |

### Análise

**Fator de carga e colisões.** No experimento B, as colisões por inserção crescem de 0,05 (α = 0,10) para 0,72 (α = 0,70), 1,54 (α = 0,90) e 2,17 (α = 0,95). Quanto mais cheia a tabela, maior a chance de a posição sorteada já estar ocupada e de a sondagem ter que continuar.

**Fator de carga e sondagens nas buscas.** As sondagens medidas acompanham de perto a teoria. Na busca malsucedida: 1,11, 1,42, 2,00, 3,33, 10,04 e 19,99 posições para α = 0,10 a 0,95 (teoria: 1,11, 1,43, 2,00, 3,33, 9,98 e 19,90). Na bem-sucedida: de 1,05 a 3,17. Isso mostra que o duplo hashing com `h1`/`h2` por extração se comporta, nestes testes, como o modelo ideal. A busca malsucedida piora muito mais que a bem-sucedida, porque só termina ao achar uma posição vazia, e elas ficam raras quando α sobe. Até α ≈ 0,7 a média é de poucas sondagens (1,72 na bem-sucedida e 3,33 na malsucedida), mas a curva dispara depois; é por isso que o limite de redimensionamento foi fixado em 0,7.

**Fator de carga e tempo.** O tempo de busca cresce bem menos que o número de sondagens: a busca malsucedida passa de 154 ns (α = 0,10) para 277 ns (α = 0,95), cerca de 1,8 vez, enquanto as sondagens crescem cerca de 18 vezes. A explicação provável é que há um custo fixo por busca (normalizar o telefone e calcular `h1` e `h2`) que não depende de α; esta explicação é uma interpretação, não foi medida separadamente. Com a tabela até α = 0,70, o tempo de busca fica praticamente estável, entre 141 e 177 ns.

**Crescimento automático (experimento A).**
- O limite de 0,7 nunca foi ultrapassado: o α final foi 0,508, 0,626 e 0,389, dependendo de onde caiu a última duplicação da capacidade. Foram 4, 7 e 11 redimensionamentos para 100, 1.000 e 10.000 registros, ou seja, a quantidade cresce de forma logarítmica porque a capacidade dobra a cada vez.
- As colisões por inserção (1,01, 1,30 e 1,56) são maiores que as do experimento B em α parecido, porque aqui elas incluem as recolocações feitas durante cada rehashing, em que todos os contatos são colocados de novo.
- O tempo médio de inserção foi de 533 ns, 463 ns e 1487 ns. O valor mais alto com 10.000 registros é compatível com o custo de copiar contatos de 420 bytes em cada rehashing e de usar vetores maiores (cerca de 10,8 MB ao final), mas não foi feito um perfil para confirmar a causa. No experimento B, os tempos de inserção mais altos com α pequeno (2150 ns em α = 0,10, com um vetor de cerca de 42 MB) também sugerem que o custo de memória pesa; isso também é uma hipótese não verificada.
- O tempo de busca ficou praticamente estável (133, 155 e 159 ns para buscas bem-sucedidas), o que é coerente com busca em tempo médio constante.

## 8. Limitações

- **Só os 7 últimos dígitos entram nos hashes.** Dois telefones que diferem apenas no início (por exemplo, DDDs diferentes com os mesmos 7 últimos dígitos) têm a mesma sequência de sondagem. Com contatos de uma região isso é raro (nos experimentos, o α real seguiu a teoria), mas uma agenda com muitos DDDs e números repetidos nos últimos 7 dígitos teria mais colisões.
- O código do país (`+55`) não é removido na normalização; se o mesmo número for digitado com e sem `+55`, será tratado como duas chaves diferentes.
- `listar_contatos` mostra os contatos na ordem do vetor, não na ordem de cadastro nem em ordem alfabética.
- A biblioteca não é segura para uso simultâneo por várias threads.
- Os tempos medidos foram obtidos em um ambiente Ubuntu 24.04 com 1 CPU; **não foram medidos no Linux Mint**, e os valores absolutos mudarão no seu computador.
