#include <stdio.h>
#include <unistd.h>

#define MAX_PAGES (256)
#define MAX_FRAMES (64)

PhysicalMemory physical_memory;
VirtualMemory virtual_memory;

/* Page: estrutura de uma pagina e seus atributos 
   'id': identificador unico de uma pagina
   'logical_address': endereco logico da pagina
   'is_in_physical_memory': identifica se a pagina esta na memoria fisica */
typedef struct {
    int id;
    int logical_address;
    int is_in_physical_memory;
} Page;

/* Frame: estrutura de um frame e seus atributos 
   'id': identificador unico de um frame
   'process_id': identificador do processo do frame
   'page_number': identificador da pagina no frame */
typedef struct {
    int id;
    int process_id;
    int page_number;
} Frame;

/* PageTable: estrutura de uma tabela de paginas e seus atributos */
typedef struct {
    int page_number;
    int frame_number;
    int is_in_physical_memory;
} PageTable;

/* Process: estrutura de um processo e seus atributos
   'pid': identificador unico do processo
   'total_pages': numero de paginas alocadas para o processo
   'page_tables': ponteiro para a tabela de paginas do processo */
typedef struct {
    int pid;
    int total_pages;
    PageTable page_tables[MAX_PAGES];
} Process;

/* PhysicalMemory: estrutura da memoria fisica e seus atributos
   'size': tamanho da memoria
   'frames': total de frames disponiveis */
typedef struct {
    int size;
    Frame frames[MAX_FRAMES];
} PhysicalMemory;

/* VirtualMemory: estrutura da memoria virtual e seus atributos
   'size': tamanho da memoria
   'pages': total de paginas disponiveis */
typedef struct {
    int size;
    Page pages[MAX_PAGES];
} VirtualMemory;

void init_physical_memory() {
    physical_memory.size = MAX_FRAMES;
    for (int i = 0; i < physical_memory.size; i++) {
        physical_memory.frames[i].id = i;
        physical_memory.frames[i].process_id = -1;
        physical_memory.frames[i].page_number = -1;
    }
}




int main() {

    int menu_option;


    while (1) {

        printf("Simulador de Paginacao\n---\n[ 1 ] Criar novo processo\nEscolha uma opcao: ");
        scanf("%d", &menu_option);

        if (menu_option > 1) {
            printf("Saindo...\n\n");
            break;
        }

        switch(menu_option) {

            case 1:

                printf();



        }



    }


    return 0;
}