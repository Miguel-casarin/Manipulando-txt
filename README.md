# Manipulando-txt
O código tem o objetivo de ler todos os arquivos txt de um diretório processa cada arquivo em uma thread separada, e gera um novo arquivo com o conteúdo em letras maiúsculas, além de analisar estatísticas sobre palavras, vogais e consoantes.

## Libs
<string.h> -> Mnipulação de strings

<pthread.h> -> Criação e gerenciamentro das threads

<ctype.h> -> Verificação e conversão de caracteres (ex: isalpha, toupper)

<dirent.h> -> Leitura de diretórios

## Código
Estrutura de nó encadeado usado na hash
```c
typedef struct NodePalavra {
    char palavra[MAX_WORD_LEN];
    int count;
    struct NodePalavra *next;
} NodePalavra;
```
Passagem de parametros para Hash
```c
typedef struct {
    char diretorio[MAX_PATH_LEN]; // caminho de destino
    char nome_arquivo[MAX_PATH_LEN]; // nome do arq
} ParamThread;

```
Listras encadeadas da tabela
```c
NodePalavra *tab_palavras[HASH_SIZE] = {NULL};
```
Função hash para definir o indice da palavra
```c
// Funções auxiliares
unsigned int hash(const char *str) {
    unsigned int hash = 0;
    while (*str) hash = (hash << 5) + *str++;
    return hash % HASH_SIZE;
}
```
A função ad_palavra isere ou atualiza uma palavra já existente na tabela Hash,
```c
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

```
onde:

```c
unsigned int indice = hash(palavra);
```
chama a função hash que calcula onde a palavra sera armagenada.
```c
NodePalavra *node = tab_palavras[indice];
```
Começa a percorrer a lista encadeada dessa posição da tabela hash.
```c
while (node) {
    if (strcmp(node->palavra, palavra) == 0) {
        node->count++;
        return;
    }
    node = node->next;
}
```
Verifica se a palavra já existe na lista encadeada
```c
NodePalavra *new_node = malloc(sizeof(NodePalavra));
strcpy(new_node->palavra, palavra);
new_node->count = 1;
new_node->next = tab_palavras[indice];
tab_palavras[indice] = new_node;
```
Se a palavra nçao foi encontrada cria um novo nó, copia a palavra definindo um contador e inserindo esse novo nó no início das listas da posição. 

A função libera_tab_palavra serve para ibera a memoria alocada para a tabela hash.
```c
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
```
A verificação de vogal e consoante e feita nas funções
```c
int verifica_vogal(char c) {
    c = tolower(c);
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
}

int verifica_consoante(char c) {
    return isalpha(c) && !verifica_vogal(c);
}
```
O processamento do arquivo e executado na função processa_arquivo_thread
```c
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
```
onde:
```c
ParamThread *param = (ParamThread *)arg;
```
Recebe os parâmetros do tipo ParamThread, contendo o caminho do diretório e o nome do arquivo a ser processado:
```c
snprintf(file_path, sizeof(file_path), "%s/%s", param->diretorio, param->nome_arquivo);
snprintf(output_path, sizeof(output_path), "%s/%s.upper.txt", param->diretorio, param->nome_arquivo);
```
Constrói os caminhos de entrada e saída:
-file_path: caminho do arquivo original a ser lido.
-output_path: caminho do novo arquivo que será criado com o conteúdo em letras maiúsculas.
```c
FILE *input = fopen(file_path, "r");
FILE *output = fopen(output_path, "w");
```
Abre os arquivos
```c
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
```
Processa caracter por caracter
```c
    if (isalpha(ch)) {
        char lower = tolower(ch);
        if (verifica_vogal(lower))
            freq_vogal[lower - 'a']++;
        else
            freq_consoante[lower - 'a']++;

        if (idx < MAX_WORD_LEN - 1)
            palavra[idx++] = lower;
    } else {
        if (idx > 0) {
            palavra[idx] = '\0';
            ad_palavra(palavra);
            count_palavras++;
            idx = 0;
        }
    }
}
```
Indentifica letra palavra, vogais e consoantes
```c
if (idx > 0) {
    palavra[idx] = '\0';
    ad_palavra(palavra);
    count_palavras++;
}
```
Processa a última palavra (se o arquivo não terminar com separador)
```c
for (int i = 0; i < 26; i++) {
    count_vogal += freq_vogal[i];
    count_consoante += freq_consoante[i];
}
```
Encontra a palavra mais frequente:
```c
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
```
Encontra a vogal e consoante mais frequente
```c
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
```
No mein
```c
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
```

-argc e argv[]: Usados para receber argumentos da linha de comando. O programa espera exatamente 1 argumento: o caminho do diretório onde estão os arquivos .txt.

-DIR *dir = opendir(argv[1]): Tenta abrir o diretório informado. Caso falhe, imprime erro e termina.

-readdir(dir): Itera sobre todos os arquivos do diretório.

-strstr(entry->d_name, ".txt") != NULL && strstr(entry->d_name, ".upper.txt") == NULL: Garante que o arquivo seja .txt e ainda não tenha sido processado (para não duplicar).

-pthread_create(...): Cria uma nova thread para processar o arquivo usando a função processa_arquivo_thread.

-pthread_join(...): Garante que o main só finalize após todas as threads terminarem.

-closedir(dir): Fecha o diretório após o processamento.

