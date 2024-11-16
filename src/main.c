/*
Integrantes:
Vitor Alves Pereira, 10410862
Eduardo Takashi Missaka, 10417877
Tiago Silveira Lopez, 10417600
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    int id;
    int frame_id;
} Page;

typedef struct
{
    int process_id;
    int total_of_pages;
    Page *pages;
} PageTable;

typedef struct
{
    int frame_id;
    int process_id;
    int page_id;
    int status;
} Frame;

typedef struct
{
    int total_of_frames;
    Frame *frames;
} PhysicalMemory;

typedef struct
{
    int process_id;
    int total_of_pages;
    PageTable page_table;
} VirtualMemory;

/* ---- */
VirtualMemory init_process(int pid, int total_of_pages)
{

    printf("Inicializando Memoria Virtual...\n");
    VirtualMemory virtual_memory;

    virtual_memory.process_id = pid;
    virtual_memory.total_of_pages = total_of_pages;

    virtual_memory.page_table.process_id = pid;
    virtual_memory.page_table.total_of_pages = total_of_pages;

    printf("Alocando %d bytes na memoria para as paginas do Processo #%d\n", total_of_pages * sizeof(Page), pid);
    virtual_memory.page_table.pages = (Page *)malloc(total_of_pages * sizeof(Page));

    if (virtual_memory.page_table.pages == NULL)
    {
        printf("ERRO!!! Nao foi possivel alocar espaco na memoria para o Processo #%d com %d paginas. Tente Novamente...\n", pid, total_of_pages);
        exit(1);
    }

    for (int i = 0; i < total_of_pages; i++)
    {
        virtual_memory.page_table.pages[i].id = i;
        virtual_memory.page_table.pages[i].frame_id = -1;
    }

    return virtual_memory;
}

void init_physical_memory(PhysicalMemory physical_memory, int total_of_frames)
{

    physical_memory.total_of_frames = total_of_frames;

    printf("Alocando %d bytes na memoria fisica para os frames...\n", total_of_frames * sizeof(Frame));
    physical_memory.frames = (Frame *)malloc(total_of_frames * sizeof(Frame));

    if (physical_memory.frames == NULL)
    {
        printf("ERRO!!! Nao foi possivel alocar espaco na memoria fisica...\n");
        exit(1);
    }

    for (int i = 0; i < total_of_frames; i++)
    {
        physical_memory.frames[i].frame_id = i;
        physical_memory.frames[i].process_id = -1;
        physical_memory.frames[i].page_id = -1;
        physical_memory.frames[i].status = 0;
    }
}

void allocate_page(PhysicalMemory *physical_memory, VirtualMemory *virtual_memory, int page_id)
{

    if (page_id >= virtual_memory->total_of_pages)
    {
        printf("ERRO!!! PID Invalido...\n");
        return;
    }

    for (int i = 0; i < physical_memory->total_of_frames; i++)
    {
        if (physical_memory->frames[i].process_id == -1)
        {

            physical_memory->frames[i].process_id = virtual_memory->process_id;
            physical_memory->frames[i].page_id = page_id;
            virtual_memory->page_table.pages[page_id].frame_id = i;

            printf("Pagina #%d Alocada no Frame #%d...\n", virtual_memory->page_table.pages[i].id, virtual_memory->page_table.pages[i].frame_id);
            return;
        }
    }

    printf("Nao ha Frame(s) disponiveis para a Pagina #%d do Processo #%d!\n", page_id, virtual_memory->process_id);
    return;
}

int main()
{

    printf("SIMULACAO DE PAGINACAO\n----------------------\n");
    printf("Lendo Arquivo de Config...\n");

    FILE *file_ptr = fopen("../../docs/config.txt", "r");

    while (file_ptr == NULL)
    {
        printf("---\nArquivo de Configuracao Nao Encontrado no caminho \"../docs/config.txt\"!!! Tente novamente inserindo o caminho manualmente...\n");
        char new_path[100];
        printf("Insira o caminho: ");
        scanf("%s", new_path);
        file_ptr = fopen(new_path, "r");
    }

    printf("Arquivo de Config Aberto com Sucesso!\n");

    int total_of_frames;
    int total_of_pages;
    int total_of_processes;
    int value;
    char buffer[100];
    char key[100];

    while (fgets(buffer, 100, file_ptr))
    {
        if (sscanf(buffer, "%s = %d", key, &value))
        {
            if (strcmp(key, "frame_size") == 0)
                total_of_frames = value;
            else if (strcmp(key, "page_size") == 0)
                total_of_pages = value;
            else if (strcmp(key, "processes") == 0)
                total_of_processes = value;
        }
    }

    printf("Inicializando Memoria Fisica...\n");
    PhysicalMemory physical_memory;
    init_physical_memory(physical_memory, total_of_frames);

    int menu_option;
    VirtualMemory running_processes[total_of_processes];

    while (1)
    {

        printf("---\nMENU:\n[ 1 ] Criar um Processo\n[ 2 ] Alocar Pagina\n[ 6 ] Sair\nEscolha uma opcao: ");
        scanf("%d", &menu_option);

        if (menu_option == 6)
        {
            printf("---\nSaindo...\n\n");
            break;
        }

        switch (menu_option)
        {

        case 1:

            int pid;
            int total_of_pages;
            int processes_idx = 0;

            printf("---\nCriando um novo processo...\n");
            printf("Insira o ID do Processo e o Total de Paginas na forma \'pid total\': ");
            scanf("%d %d", &pid, &total_of_pages);

            while (pid < 0 && total_of_pages <= 0)
            {
                printf("ERRO!!! Nao e possivel inserir valores menores que zero para o ID do Processo ou inserir valores menores ou iguais a zero para o Total de Paginas. Tente Novamente...\n");
                printf("Insira o ID do Processo e o Total de Paginas na forma \'pid total\': ");
                scanf("%d %d", &pid, &total_of_pages);
            }

            running_processes[processes_idx] = init_process(pid, total_of_pages);

            printf("Processo #%d de %d paginas criado com sucesso!\n", pid, total_of_pages);

            processes_idx++;
            break;

        case 2:

            printf("---\nAlocando as Paginas do Processo Criado em Frames\n");

            int process_id;
            int page_id;

            printf("Insira o ID do Processo seguido pelo ID da Pagina na forma \'process_id page_id\': ");
            scanf("%d %d", &process_id, &page_id);

            for (int i = 0; i < total_of_processes; i++)
            {
                if (running_processes[i].process_id == process_id)
                {
                    allocate_page(&physical_memory, &running_processes[i], page_id);
                    break;
                }
            }

            break;

        default:
            printf("---\nOpcao Invalida!!! Tente Novamente apenas com valores de 1 a 6...\n");
            break;
        }
    }

    return 0;
}