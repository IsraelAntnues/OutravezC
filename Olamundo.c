#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXNOME 64
#define MAXPISTA 128
#define TAM_HASH 31

/* -------------------- ESTRUTURAS -------------------- */

// Estrutura da sala (nó da árvore binária)
typedef struct Sala {
    char nome[MAXNOME];
    char pista[MAXPISTA];
    char suspeito[MAXNOME]; // suspeito associado à pista da sala
    struct Sala *esq, *dir;
} Sala;

// Nó da BST de pistas coletadas
typedef struct PistaNode {
    char pista[MAXPISTA];
    struct PistaNode *esq, *dir;
} PistaNode;

// Nó da tabela hash (lista encadeada para tratamento de colisões)
typedef struct HashNode {
    char pista[MAXPISTA];
    char suspeito[MAXNOME];
    struct HashNode *prox;
} HashNode;

/* Tabela hash global */
HashNode* tabelaHash[TAM_HASH];

/* Árvore BST de pistas coletadas */
PistaNode *raizPistas = NULL;

/* -------------------- PROTÓTIPOS -------------------- */

// criaSala() – cria dinamicamente um cômodo.
Sala* criarSala(const char *nome, const char *pista, const char *suspeito);

// explorarSalas() – navega pela árvore e ativa o sistema de pistas.
void explorarSalas(Sala *raiz);

// inserirPista() / adicionarPista() – insere a pista coletada na árvore de pistas.
PistaNode* inserirPista(PistaNode *raiz, const char *pista, int *inseriu);
int adicionarPista(const char *pista);
void listarPistas(PistaNode *raiz);

// inserirNaHash() – insere associação pista/suspeito na tabela hash.
unsigned int hash(const char *str);
void inserirNaHash(const char *pista, const char *suspeito);

// encontrarSuspeito() – consulta o suspeito correspondente a uma pista.
char* encontrarSuspeito(const char *pista);

// verificarSuspeitoFinal() – conduz à fase de julgamento final.
void verificarSuspeitoFinal(const char *acusacao);

// funções utilitárias
void liberarSalas(Sala *raiz);
void liberarPistas(PistaNode *raiz);
void liberarHash();

/* -------------------- IMPLEMENTAÇÕES -------------------- */

// Cria dinamicamente um cômodo com nome, pista e suspeito.
Sala* criarSala(const char *nome, const char *pista, const char *suspeito) {
    Sala *s = (Sala*) malloc(sizeof(Sala));
    if (!s) {
        perror("Erro ao alocar sala");
        exit(EXIT_FAILURE);
    }
    strncpy(s->nome, nome, MAXNOME-1);
    s->nome[MAXNOME-1] = '\0';
    if (pista) {
        strncpy(s->pista, pista, MAXPISTA-1);
        s->pista[MAXPISTA-1] = '\0';
    } else {
        s->pista[0] = '\0';
    }
    if (suspeito) {
        strncpy(s->suspeito, suspeito, MAXNOME-1);
        s->suspeito[MAXNOME-1] = '\0';
    } else {
        s->suspeito[0] = '\0';
    }
    s->esq = s->dir = NULL;
    return s;
}

// Inserção em BST de pistas (ordem alfabética). Retorna nova raiz.
PistaNode* inserirPista(PistaNode *raiz, const char *pista, int *inseriu) {
    if (!raiz) {
        PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) { perror("Erro ao alocar pista"); exit(EXIT_FAILURE); }
        strncpy(n->pista, pista, MAXPISTA-1);
        n->pista[MAXPISTA-1] = '\0';
        n->esq = n->dir = NULL;
        *inseriu = 1;
        return n;
    }
    int cmp = strcmp(pista, raiz->pista);
    if (cmp == 0) { // já existe
        *inseriu = 0;
        return raiz;
    } else if (cmp < 0) {
        raiz->esq = inserirPista(raiz->esq, pista, inseriu);
    } else {
        raiz->dir = inserirPista(raiz->dir, pista, inseriu);
    }
    return raiz;
}

// Wrapper para adicionar pista à árvore global
int adicionarPista(const char *pista) {
    int inseriu = 0;
    raizPistas = inserirPista(raizPistas, pista, &inseriu);
    return inseriu;
}

// Listar pistas em ordem (inorder)
void listarPistas(PistaNode *raiz) {
    if (!raiz) return;
    listarPistas(raiz->esq);
    printf("- %s\n", raiz->pista);
    listarPistas(raiz->dir);
}

// Função hash simples (djb2)
unsigned int hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return (unsigned int)(hash % TAM_HASH);
}

// Insere pista -> suspeito na tabela hash
void inserirNaHash(const char *pista, const char *suspeito) {
    unsigned int idx = hash(pista);
    // verificar se já existe a entrada para evitar duplicatas
    HashNode *atual = tabelaHash[idx];
    while (atual) {
        if (strcmp(atual->pista, pista) == 0) return; // já mapeado
        atual = atual->prox;
    }
    HashNode *n = (HashNode*) malloc(sizeof(HashNode));
    if (!n) { perror("Erro ao alocar HashNode"); exit(EXIT_FAILURE); }
    strncpy(n->pista, pista, MAXPISTA-1);
    n->pista[MAXPISTA-1] = '\0';
    strncpy(n->suspeito, suspeito, MAXNOME-1);
    n->suspeito[MAXNOME-1] = '\0';
    n->prox = tabelaHash[idx];
    tabelaHash[idx] = n;
}

// Encontrar suspeito a partir de uma pista
char* encontrarSuspeito(const char *pista) {
    unsigned int idx = hash(pista);
    HashNode *atual = tabelaHash[idx];
    while (atual) {
        if (strcmp(atual->pista, pista) == 0) return atual->suspeito;
        atual = atual->prox;
    }
    return NULL;
}

// Conta quantas pistas na BST apontam para o suspeito dado
int contarPistasParaSuspeito(PistaNode *raiz, const char *acusado) {
    if (!raiz) return 0;
    int count = 0;
    char *s = encontrarSuspeito(raiz->pista);
    if (s && strcmp(s, acusado) == 0) count = 1;
    return count + contarPistasParaSuspeito(raiz->esq, acusado) + contarPistasParaSuspeito(raiz->dir, acusado);
}

// verificarSuspeitoFinal() – conduz à fase de julgamento final.
void verificarSuspeitoFinal(const char *acusacao) {
    int total = contarPistasParaSuspeito(raizPistas, acusacao);
    printf("\nResultado do julgamento para '%s':\n", acusacao);
    if (total >= 2) {
        printf("%d pista(s) suportam a acusação. Você resolveu o mistério!\n", total);
    } else if (total == 1) {
        printf("Apenas 1 pista aponta para %s. Acusação insuficiente.\n", acusacao);
    } else {
        printf("Nenhuma pista aponta para %s. Acusação incorreta.\n", acusacao);
    }
}

// Explorar salas interativamente
void explorarSalas(Sala *raiz) {
    Sala *atual = raiz;
    char escolha[8];
    char buffer[128];

    while (atual) {
        printf("\nVocê está na sala: %s\n", atual->nome);
        if (strlen(atual->pista) > 0) {
            printf("Pista encontrada: %s\n", atual->pista);
            // adiciona à BST (evita duplicatas)
            if (adicionarPista(atual->pista)) {
                printf("Pista adicionada à coleção.\n");
                // insere na hash a associação pista -> suspeito
                if (strlen(atual->suspeito) > 0) inserirNaHash(atual->pista, atual->suspeito);
            } else {
                printf("Você já coletou essa pista antes.\n");
            }
        } else {
            printf("Nenhuma pista nesta sala.\n");
        }

        printf("Escolha o caminho: (e) esquerda | (d) direita | (s) sair\n");
        printf("> ");
        if (!fgets(escolha, sizeof(escolha), stdin)) break;
        // remover newline
        escolha[strcspn(escolha, "\n")] = '\0';

        if (strcmp(escolha, "e") == 0) {
            if (atual->esq) atual = atual->esq;
            else printf("Nao ha sala à esquerda.\n");
        } else if (strcmp(escolha, "d") == 0) {
            if (atual->dir) atual = atual->dir;
            else printf("Nao ha sala à direita.\n");
        } else if (strcmp(escolha, "s") == 0) {
            printf("Saindo da exploração...\n");
            break;
        } else {
            printf("Opcao invalida. Use 'e', 'd' ou 's'.\n");
        }
    }

    // Exploração terminou — lista pistas coletadas
    printf("\nPistas coletadas (ordenadas):\n");
    if (!raizPistas) printf("(nenhuma pista coletada)\n");
    else listarPistas(raizPistas);

    // solicitar acusação
    printf("\nDigite o nome do suspeito para acusar: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    buffer[strcspn(buffer, "\n")] = '\0';

    verificarSuspeitoFinal(buffer);
}

/* Funções de liberação de memoria */
void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esq);
    liberarSalas(raiz->dir);
    free(raiz);
}

void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz);
}

void liberarHash() {
    for (int i = 0; i < TAM_HASH; ++i) {
        HashNode *cur = tabelaHash[i];
        while (cur) {
            HashNode *tmp = cur;
            cur = cur->prox;
            free(tmp);
        }
        tabelaHash[i] = NULL;
    }
}

/* -------------------- MAIN: monta o mapa fixo da mansao -------------------- */
int main() {
    // inicializar tabela hash
    for (int i = 0; i < TAM_HASH; ++i) tabelaHash[i] = NULL;

    // Montagem manual do mapa (simplificacao para o nivel mestre)
    // Raiz
    Sala *hall = criarSala("Hall", "pegada no tapete", "Sra. White");
    // Nivel 1
    Sala *cozinha = criarSala("Cozinha", "faca com manchas", "Sr. Black");
    Sala *escritorio = criarSala("Escritorio", "bilhete rasgado", "Sra. Green");
    hall->esq = cozinha;
    hall->dir = escritorio;
    // Nivel 2
    Sala *despensa = criarSala("Despensa", "luvas sujas", "Sr. Black");
    Sala *jardim = criarSala("Jardim", "impressao digital", "Sra. White");
    cozinha->esq = despensa;
    cozinha->dir = jardim;

    Sala *biblioteca = criarSala("Biblioteca", "marca de xicara", "Sra. Green");
    escritorio->dir = biblioteca;

    printf("Bem-vindo ao jogo de detetive! Explore a mansao e colete pistas.\n");
    explorarSalas(hall);

    // liberar memoria
    liberarSalas(hall);
    liberarPistas(raizPistas);
    liberarHash();

    printf("\nObrigado por jogar!\n");
    return 0;
}


