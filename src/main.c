#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int id;
    int total_of_frames;
} Page;

typedef struct {
    int process_id;
    int total_of_pages;
    Page *pages;
} PageTable;

typedef struct {
    int frame_id;
    int process_id;
    int page_id;
    int status;
} Frame;

typedef struct {
    int total_of_frames;
    Frame *frames;
} PhysicalMemory;

typedef struct {
    int process_id;
    int total_of_pages;
    PageTable page_table;
} VirtualMemory;

/* ---- */
void init_physical_memory() {





}

void clean_char_arr(char arr[], int arr_size) {
    for (int i = 0; i < arr_size; i++)
        arr[i] = '\0';
}

int main() {

    printf("SIMULACAO DE PAGINACAO\n----------------------\n");
    printf("Inicializando Memoria Fisica...\n");
    printf("Lendo Arquivo de Config...\n");

    FILE *file_ptr = fopen("../../docs/config.txt", "r");

    while (file_ptr == NULL) {
        printf("Arquivo de Configuracao Nao Encontrado no caminho \"../docs/config.txt\"!!! Tente novamente inserindo o caminho manualmente...\n");
        char new_path[100];
        printf("Insira o caminho: ");
        scanf("%s", new_path);
        file_ptr = fopen(new_path, "r");
    }

    printf("Arquivo de Config Lido com Sucesso!\n");

    int frames = 0, pages = 0, processes = 0, value = 0;
    char buffer[100], key[100];

    while (fgets(buffer, 100, file_ptr)) {
        if (sscanf(buffer, "%s = %d", key, &value)) {
            if (strcmp(key, "frame_size") == 0) frames = value;
            else if (strcmp(key, "page_size") == 0) pages = value;
            else if (strcmp(key, "processes") == 0) processes = value;
        }
    }

    printf("Frames: %d\nPages: %d\nProcesses: %d", frames, pages, processes);


    return 0;
}