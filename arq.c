//Para executar e precisso rodar o arquivo compilado junto ao diretório "./final1 /mnt/c/sistemas/threads/ex5"


// Funções utilizadas da <ctype.h>:
//
// isalpha(c):
//   Verifica se 'c' é uma letra (A-Z ou a-z). Retorna diferente de zero se for verdadeiro.
//
// tolower(c):
//   Converte 'c' para minúscula, se for uma letra maiúscula. Caso contrário, retorna o próprio caractere.
//
// toupper(c):
//   Converte 'c' para maiúscula, se for uma letra minúscula. Caso contrário, retorna o próprio caractere.


// Funções/estruturas utilizadas da <dirent.h>:
//
// DIR *opendir(const char *nome):
//   Abre o diretório especificado e retorna um ponteiro para ele. Retorna NULL em caso de erro.
//
// struct dirent *readdir(DIR *dir):
//   Lê sequencialmente os arquivos/entradas dentro do diretório. Retorna NULL quando não há mais arquivos.
//
// void closedir(DIR *dir):
//   Fecha o diretório previamente aberto com opendir.
//
// entry->d_name:
//   Campo da struct dirent que contém o nome do arquivo atual lido por readdir.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h> // lib usada para manipulara caracteres
#include <dirent.h> // lib usada para manipular diretorios 

#define MAX_WORD_LEN 1000 // defino o tamanho max palavra
#define HASH_SIZE 1000 // DEFINO O TAMANHO MAX DA TAB HASH
#define MAX_THREADS 1000 // define o máximo para  pre alocar o "pthread_t threads[MAX_THREADS];"
#define MAX_PATH_LEN 512 // define o tamanho max do path

// armazena na hash
typedef struct NodePalavra {
    char palavra[MAX_WORD_LEN];
    int count;
    struct NodePalavra *next; // ponteiro para o proximo
} NodePalavra;

// passa parametros thread
typedef struct {
    char diretorio[MAX_PATH_LEN]; // caminho de destino
    char nome_arquivo[MAX_PATH_LEN]; // nome do arq
} ParamThread;

NodePalavra *tab_palavras[HASH_SIZE] = {NULL};

// Funções auxiliares
unsigned int hash(const char *str) {
    unsigned int hash = 0;
    while (*str) hash = (hash << 5) + *str++;
    return hash % HASH_SIZE;
}

// ad palavra a tabela
void ad_palavra(const char *palavra) {
    unsigned int indice = hash(palavra);
    NodePalavra *node = tab_palavras[indice];
    // Verifica se a palavra já existe
    while (node) {
        if (strcmp(node->palavra, palavra) == 0) {
            node->count++;
            return;
        }
        node = node->next;
    }

    // Cria novo nó se a palavra não existir
    NodePalavra *new_node = malloc(sizeof(NodePalavra));
    strcpy(new_node->palavra, palavra);
    new_node->count = 1;
    new_node->next = tab_palavras[indice];
    tab_palavras[indice] = new_node;
}

// libera memoria alocada para a tabela hash
void libera_tab_palavra() {
    for (int i = 0; i < HASH_SIZE; i++) {
        NodePalavra *node = tab_palavras[i];
        while (node) {
            NodePalavra *tmp = node;
            node = node->next;
            free(tmp);
        }
        tab_palavras[i] = NULL;
    }
}

int verifica_vogal(char c) {
    c = tolower(c);
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
}

int verifica_consoante(char c) {
    return isalpha(c) && !verifica_vogal(c);
}

// aqui e executado todo o processamento do arq
void *processa_arquivo_thread(void *arg) {
    ParamThread *param = (ParamThread *)arg;
    char file_path[3024], output_path[3024];
    // Monta caminho do arquivo de entrada
    snprintf(file_path, sizeof(file_path), "%s/%s", param->diretorio, param->nome_arquivo);
    // Monta caminho do arquivo de saída
    snprintf(output_path, sizeof(output_path), "%s/%s.upper.txt", param->diretorio, param->nome_arquivo);

    // abre arq de entrada
    FILE *input = fopen(file_path, "r");
    if (!input) {
        perror("Erro ao abrir arquivo");
        free(param);
        return NULL;
    }
    // cria arquivo de saida
    FILE *output = fopen(output_path, "w");
    if (!output) {
        perror("Erro ao criar arquivo de saída");
        fclose(input);
        free(param);
        return NULL;
    }

    // var para contagem
    int count_palavras = 0, count_vogal = 0, count_consoante = 0;
    int freq_vogal[26] = {0}, freq_consoante[26] = {0};
    char ch, palavra[MAX_WORD_LEN];
    int idx = 0;

    // Lê caractere por caractere do arquivo
    while ((ch = fgetc(input)) != EOF) {
        fputc(toupper(ch), output); // escreve a versão maiúscula no arquivo de saída
        if (isalpha(ch)) {
            char lower = tolower(ch);  // converte para minúscula para contagem
            if (verifica_vogal(lower))
                freq_vogal[lower - 'a']++;
            else
                freq_consoante[lower - 'a']++;

            if (idx < MAX_WORD_LEN - 1)
                palavra[idx++] = lower;
        } else {
            if (idx > 0) {
                palavra[idx] = '\0';
                ad_palavra(palavra); // chama func que adiciona a palavra
                count_palavras++;
                idx = 0;
            }
        }
    }
     // Trata a última palavra, se o arquivo não terminar com separador
    if (idx > 0) {
        palavra[idx] = '\0';
        ad_palavra(palavra);
        count_palavras++;
    }
    // contagem de vogais e consoantes
    for (int i = 0; i < 26; i++) {
        count_vogal += freq_vogal[i];
        count_consoante += freq_consoante[i];
    }

    // palavra mais frequente
    char palavra_mais_freq[MAX_WORD_LEN] = "";
    int max_pcount = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        NodePalavra *node = tab_palavras[i];
        while (node) {
            if (node->count > max_pcount) {
                max_pcount = node->count;
                strcpy(palavra_mais_freq, node->palavra);
            }
            node = node->next;
        }
    }

    // vogal e consoante mais frequente
    char vogal_mais_freq = '\0', consoante_mais_freq = '\0';
    int max_vogal = 0, max_consoante = 0;
    for (int i = 0; i < 26; i++) {
        if (freq_vogal[i] > max_vogal) {
            max_vogal = freq_vogal[i];
            vogal_mais_freq = 'a' + i;
        }
        if (freq_consoante[i] > max_consoante) {
            max_consoante = freq_consoante[i];
            consoante_mais_freq = 'a' + i;
        }
    }

    printf("Arquivo: %s\n", param->nome_arquivo);
    printf("Número de palavras: %d\n", count_palavras);
    printf("Número de vogais: %d\n", count_vogal);
    printf("Número de consoantes: %d\n", count_consoante);
    printf("Palavra mais frequente: %s (%d vezes)\n", palavra_mais_freq, max_pcount);
    printf("Vogal mais frequente: %c (%d vezes)\n", vogal_mais_freq, max_vogal);
    printf("Consoante mais frequente: %c (%d vezes)\n\n", consoante_mais_freq, max_consoante);

    // libera memoria 
    fclose(input);
    fclose(output);
    libera_tab_palavra();
    free(param);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <diretorio>\n", argv[0]);
        return 1;
    }

    DIR *dir = opendir(argv[1]);
    if (!dir) {
        perror("Erro ao abrir diretório");
        return 1;
    }

    struct dirent *entry; // abre diretório fornecido na linha de comando
    pthread_t threads[MAX_THREADS]; // array de threads
    int thread_count = 0;

    // Itera sobre os arquivos no diretório
    while ((entry = readdir(dir)) != NULL) {
         // Só processa arquivos que terminam com .txt e NÃO com .upper.txt
        if (strstr(entry->d_name, ".txt") != NULL &&
            strstr(entry->d_name, ".upper.txt") == NULL) {
            
            if (thread_count >= MAX_THREADS) {
                fprintf(stderr, "Limite de threads excedido!\n");
                break;
            }

            // Prepara parâmetros para a thread
            ParamThread *param = malloc(sizeof(ParamThread));
            strcpy(param->diretorio, argv[1]);
            strcpy(param->nome_arquivo, entry->d_name);

            // Cria a thread para processar o arquivo
            pthread_create(&threads[thread_count], NULL, processa_arquivo_thread, param);
            thread_count++;
        }
    }

     // Aguarda todas as threads terminarem
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    closedir(dir);
    return 0;
}
