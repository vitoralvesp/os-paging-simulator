#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//ESTRUTURA DA PAGINA
typedef struct
{
    int id;
    int frame_id;
} Page;

//ESTRUTURA DA TABELA DE PAGINAS
typedef struct
{
    int process_id;
    int total_of_pages;
    Page *pages;
} PageTable;

//ESTRUTURA DO FRAME
typedef struct
{
    int frame_id;
    int process_id;
    int page_id;
    int status;
} Frame;

//ESTRUTURA DA MEMORIA FISICA
typedef struct
{
    int total_of_frames;
    Frame *frames;
} PhysicalMemory;

//ESTRUTURA DO PROCESSO (adicionada recentemente)
typedef struct {
    int process_id;
    int total_of_pages;
    Page *pages;  // Tabela de páginas associada ao processo
} Processo;


//ESTRUTURA DA MEMORIA VIRTUAL
typedef struct
{
    int process_id;
    int total_of_pages;
    PageTable page_table;
} VirtualMemory;

/* ---- */
//CRIAÇAO DO ARQUIVO .LOG (há vários trechos do código onde são gradualmente inseridos mensagens relevantes perante a seu comportamento)
void log_operation(const char *message)
{
    FILE *log_file = fopen("simulation.log", "a");
    if (log_file == NULL)
    {
        printf("ERRO!!! Nao foi possível criar o arquivo de log.\n");
        exit(1);
    }
    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

//INICIAR PROCESSO
VirtualMemory init_process(int pid, int total_of_pages)
{
int i;
    printf("Inicializando processo na memoria virtual...\n");
    VirtualMemory virtual_memory;
    virtual_memory.process_id = pid; //Usuário informa id do processo no caso 1
    virtual_memory.total_of_pages = total_of_pages; //Usuário informa total de páginas no caso 1

    virtual_memory.page_table.process_id = pid; //Adiciona ID do processo para a tabela de paginas
    virtual_memory.page_table.total_of_pages = total_of_pages; //Adiciona total de paginas para a tabela de paginas

    printf("Alocando %d bytes na memoria para as paginas do Processo #%d\n", total_of_pages * sizeof(Page), pid);
    virtual_memory.page_table.pages = (Page *)malloc(total_of_pages * sizeof(Page));
    //Caso usuário informe 0 páginas
    if (virtual_memory.page_table.pages == NULL)
    {
        printf("ERRO!!! Nao foi possivel alocar espaco na memoria para o Processo #%d com %d paginas. Tente Novamente...\n", pid, total_of_pages);
        exit(1);
    }
    //Inicialização de página por página na tabela de páginas da memória virtual: cada página recebe seu ID e seu frame_id
    for (i = 0; i < total_of_pages; i++)
    {
        virtual_memory.page_table.pages[i].id = i;
        virtual_memory.page_table.pages[i].frame_id = -1;
    }

    // LOG GERADO
    char log_msg[100];
    sprintf(log_msg, "PROCESS_CREATED: Processo #%d com %d paginas inicializado.", pid, total_of_pages);
    log_operation(log_msg);

    return virtual_memory;
}

//INICIAR MEMORIA FISICA
void init_physical_memory(PhysicalMemory *physical_memory, int total_of_frames)
{
    int i;
    physical_memory->total_of_frames = total_of_frames;
    //Alocação
    printf("Alocando %d bytes na memoria fisica para os frames...\n", total_of_frames * sizeof(Frame));
    physical_memory->frames = (Frame *)malloc(total_of_frames * sizeof(Frame));

    if (physical_memory->frames == NULL) //Não há frames? erro.
    {
        printf("ERRO!!! Nao foi possivel alocar espaco na memoria fisica...\n");
        exit(1);
    }
    
    //Inicialização de cada frame da memoria fisica, levando em conta o total de frames
    for (i = 0; i < total_of_frames; i++)
    {
        physical_memory->frames[i].frame_id = i;
        physical_memory->frames[i].process_id = -1;
        physical_memory->frames[i].page_id = -1;
        physical_memory->frames[i].status = 0;
    }
    
    // LOG GERADO
    char log_msg[100];
    sprintf(log_msg, "PHYSICAL_MEMORY_INITIALIZED: %d frames alocados.", total_of_frames);
    log_operation(log_msg);
}

//PAGE FAULT
void handle_page_fault(PhysicalMemory *physical_memory, VirtualMemory *virtual_memory, int page_id, int sleep_time)
{
    printf("PAGEFAULT001 - Pagina #%d do Processo #%d\n", page_id, virtual_memory->process_id);
    // LOG GERADO
    log_operation("PAGEFAULT001");
    //Deve buscar na memoria secundaria
    sleep(sleep_time);

    //Substituição aleatória
    int victim_frame = rand() % physical_memory->total_of_frames;

    //Substituir página no frame escolhido aleatoriamente
    Frame *frame = &physical_memory->frames[victim_frame];
    //Caso o frame esteja ocupado, deve substituir pela página atual
    if (frame->process_id != -1)
    {
        VirtualMemory *victim_process = virtual_memory;
        victim_process->page_table.pages[frame->page_id].frame_id = -1; //Reiniciar valor para -1
        
        // LOG GERADO
        char log_msg[100];
        sprintf(log_msg, "Substituindo Pagina #%d do Processo #%d no Frame #%d",
                frame->page_id, frame->process_id, victim_frame);
        log_operation(log_msg);
    }

    frame->process_id = virtual_memory->process_id; //Frame recebe 'process_id' para saber de qual processo é
    frame->page_id = page_id; //Frame recebe o id da página a qual está associado
    virtual_memory->page_table.pages[page_id].frame_id = victim_frame;
    printf("Pagina #%d alocada no Frame #%d\n", page_id, victim_frame);
    
    // LOG GERADO
    char log_msg[100];
    sprintf(log_msg, "PAGE_ALLOCATED: Pagina #%d do Processo #%d alocada no Frame #%d.",
            page_id, virtual_memory->process_id, victim_frame);
    log_operation(log_msg);
}

//TRADUÇÃO DE ENDEREÇO VIRTUAL PARA FÍSICO
int translate_address(VirtualMemory *virtual_memory, int page_id)
{
    //Verifica se o page_id está dentro do intervalo válido (obs: provavelmente não utilizado, pois o check já é feito em 'allocate_page')
    if (page_id < 0 || page_id >= virtual_memory->total_of_pages)
    {
        printf("ERRO!!! Pagina #%d esta fora do intervalo válido (0-%d).\n", 
               page_id, virtual_memory->total_of_pages - 1);
        return -1;
    }

    //Obter o frame_id associado à página
    int frame_id = virtual_memory->page_table.pages[page_id].frame_id;

    // Depuração: mostra o estado atual da página
    printf("Debug: translate_address - Processo #%d, Pagina #%d, Frame ID = %d\n",
           virtual_memory->process_id, page_id, frame_id);

    return frame_id; // Retorna o frame_id (ou -1 se não estiver mapeado)
}



void allocate_page(PhysicalMemory *physical_memory, VirtualMemory *virtual_memory, int page_id, int sleep_time)
{
    // Verifica se o page_id é válido: deve estar entre 0 e ('total_of_pages' - 1)
    if (page_id < 0 || page_id >= virtual_memory->total_of_pages)
    {
        printf("ERRO!!! ID de Pagina invalido: %d. Intervalo permitido: 0-%d.\n", 
               page_id, virtual_memory->total_of_pages - 1);
        return;
    }

    // Página já está alocada??
    Page *page = &virtual_memory->page_table.pages[page_id]; //Obter página
    if (page->frame_id != -1) { // Página já mapeada para um frame
        printf("Pagina #%d do Processo #%d já esta alocada no Frame #%d.\n", page_id, virtual_memory->process_id, page->frame_id);
        return;
    }
    
    // Verifica se a memória física foi inicializada
    if (physical_memory->frames == NULL)
    {
        printf("ERRO!!! Memoria fisica não inicializada.\n");
        return;
    }

    // Traduz o endereço lógico (page_id) para o endereço físico (frame_id)
    int frame_id = translate_address(virtual_memory, page_id);

    // Depuração: resultado da tradução
    printf("Debug: allocate_page - Processo #%d, Pagina #%d, Frame ID retornado = %d\n",
           virtual_memory->process_id, page_id, frame_id);

    // Se a tradução falhou (não encontrou um frame disponível), ocorre um page fault
    if (frame_id == -1)
    {
        printf("PAGE FAULT! Pagina #%d do Processo #%d nao esta mapeada para nenhum Frame.\n", 
               page_id, virtual_memory->process_id);
        
        handle_page_fault(physical_memory, virtual_memory, page_id, sleep_time);

        // Após tratar o page fault, novamente tenta traduzir
        frame_id = translate_address(virtual_memory, page_id);

        if (frame_id != -1)
        {
            printf("Pagina #%d do Processo #%d mapeada para Frame #%d após Page Fault.\n",
                   page_id, virtual_memory->process_id, frame_id);
        }
        else
        {
            printf("ERRO!!! Pagina #%d do Processo #%d ainda não esta mapeada apos Page Fault.\n",
                   page_id, virtual_memory->process_id);
        }
    }

    //Página mapeada, alocando no frame correspondente
    Frame *frame = &physical_memory->frames[frame_id];
    frame->process_id = virtual_memory->process_id;
    frame->page_id = page_id;
    frame->status = 1; // Frame ocupado

    //Atualizar a tabela de páginas
    virtual_memory->page_table.pages[page_id].frame_id = frame_id;

    //Impressão
    printf("Pagina #%d do Processo #%d alocada no Frame #%d.\n", 
           page_id, virtual_memory->process_id, frame_id);

    //Simular delay
    sleep(sleep_time);
    
    //LOG GERADO
    char log_msg[100];
    sprintf(log_msg, "PAGE_ALLOCATED: Pagina #%d do Processo #%d alocada com sucesso no Frame #%d.",
            page_id, virtual_memory->process_id, frame_id);
    log_operation(log_msg);
}

//DESTRUIR PROCESSO
void terminate_process(PhysicalMemory *physical_memory, VirtualMemory *virtual_memory) {
	int i;
    printf("Finalizando Processo #%d...\n", virtual_memory->process_id);

    //Liberar os frames ocupados pelo processo na memória física
    for (i = 0; i < virtual_memory->total_of_pages; i++) {
        int frame_id = virtual_memory->page_table.pages[i].frame_id; //Obter possíveis frames mapeados para páginas do processo a ser exterminado
        if (frame_id != -1) { //Página está mapeada para um frame?
            physical_memory->frames[frame_id].process_id = -1; //Reinicializar variável
            physical_memory->frames[frame_id].page_id = -1; //Reinicializar variável
            physical_memory->frames[frame_id].status = 0; // Frame liberado
        }
    }

    //Liberar e reinicializar váriaveis da memória virtual do processo
    free(virtual_memory->page_table.pages);
    virtual_memory->page_table.pages = NULL;
    virtual_memory->total_of_pages = 0;
    virtual_memory->process_id = -1;

    printf("Processo #%d finalizado com sucesso.\n", virtual_memory->process_id);
    
    //LOG GERADO
    char log_msg[100];
    sprintf(log_msg, "PROCESS_TERMINATED: Processo #%d exterminado.",virtual_memory->process_id);
    log_operation(log_msg);
}

//MOSTRAR MEMORIA FÍSICA (LISTAR TODOS OS FRAMES)
void display_physical_memory_status(PhysicalMemory *physical_memory) {
	int i;
    printf("\n--- Status da Memoria Fisica ---\n");
    for (i = 0; i < physical_memory->total_of_frames; i++) {
        Frame *frame = &physical_memory->frames[i];
        printf("Frame #%d: ", frame->frame_id);
        if (frame->status == 0) {
            printf("LIVRE\n");
        } else {
            printf("OCUPADO (Processo #%d, Pagina #%d)\n", frame->process_id, frame->page_id);
        }
    }
    printf("---------------------------------\n");
}

//MOSTRAR MEMORIA VIRTUAL (LISTAR TODAS PAGINAS DO PROCESSO)
void display_virtual_memory_status(VirtualMemory *virtual_memory) {
	int i;
    if (virtual_memory->process_id == -1 || virtual_memory->page_table.pages == NULL) {
        printf("Nenhum processo carregado na memoria virtual.\n");
        return;
    }

    printf("\n--- Status da Memoria Virtual (Processo #%d) ---\n", virtual_memory->process_id);
    for (i = 0; i < virtual_memory->total_of_pages; i++) {
        int frame_id = virtual_memory->page_table.pages[i].frame_id;
        printf("Pagina #%d: ", virtual_memory->page_table.pages[i].id);
        if (frame_id == -1) {
            printf("NAO MAPEADA\n");
        } else {
            printf("Mapeada para Frame #%d\n", frame_id);
        }
    }
    printf("-----------------------------------------------\n");
}

//VALIDAR CONFIGURAÇÕES
int validate_configuration(int total_of_frames, int total_of_pages, int total_of_processes, int sleep_time) {
    if (total_of_frames <= 0) {
        printf("ERRO: Tamanho de frame invalido! Deve ser maior que zero.\n");
        return 0;
    }

    if (total_of_pages <= 0) {
        printf("ERRO: Tamanho de pagina invalido! Deve ser maior que zero.\n");
        return 0;
    }

    if (total_of_processes <= 0) {
        printf("ERRO: Número de processos invalido! Deve ser maior que zero.\n");
        return 0;
    }

    if (sleep_time < 0) {
        printf("ERRO: Tempo de espera invalido! Deve ser zero ou maior.\n");
        return 0; 
    }

    return 1;  //Todos os parâmetros são válidos
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
    
    int i;
    int value;
    char buffer[100];
    char key[100];
    int pid;
    int processes_idx = 0;
    int process_id;
    int page_id;
    //CONFIGURAÇÕES INICIAIS
    int total_of_frames = 0;
    int total_of_pages = 0;
    int total_of_processes = 0;
    int sleep_time = 0;

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
            else if (strcmp(key, "sleep_time") == 0)
                sleep_time = value;
        }
    }
    
    fclose(file_ptr);
    
    if (!validate_configuration(total_of_frames, total_of_pages, total_of_processes, sleep_time)) {
    //A configuração falhou, encerrar programa
    exit(1);
}
    
    printf("Inicializando Memoria Fisica...\n");
    PhysicalMemory physical_memory;
    init_physical_memory(&physical_memory, total_of_frames);

    int menu_option;
    VirtualMemory running_processes[total_of_processes];

    while (1)
    {

        printf("---\nMENU:\n");
        printf("[ 1 ] Criar um Processo\n");
        printf("[ 2 ] Alocar Pagina\n");
        printf("[ 3 ] Finalizar um Processo\n");
        printf("[ 4 ] Exibir Status da Memoria Fisica\n");
        printf("[ 5 ] Exibir Status da Memoria Virtual\n");
        printf("[ 6 ] Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &menu_option);

        if (menu_option == 6)
        {
            printf("---\nSaindo...\n\n");
            break;
        }

        switch (menu_option)
        {

        case 1: //Criar processo
            if (processes_idx >= total_of_processes)
            {
                printf("ERRO!!! Limite de processos atingido.\n");
                break;
            }
            int pid, total_pages;
            printf("Insira o ID do Processo e o Total de Páginas (pid total): ");
            scanf("%d %d", &pid, &total_pages);
            
            if(total_of_pages <= 0) {
            	printf("Paginas deve ser maior que zero!");
			}
			else {
            running_processes[processes_idx] = init_process(pid, total_pages);
            printf("Processo #%d criado com %d paginas.\n", pid, total_pages);
            processes_idx++;
            }
            break;

        case 2: //Alocar pagina
            printf("Insira o ID do Processo e o ID da Pagina (process_id page_id): ");
            int process_id, page_id;
            scanf("%d %d", &process_id, &page_id);

            int found = 0;
            for (i = 0; i < processes_idx; i++)
            {
                if (running_processes[i].process_id == process_id)
                {
                    allocate_page(&physical_memory, &running_processes[i], page_id, sleep_time);
                    found = 1;
            }
        }
                    if (!found)
					{
                    printf("ERRO!!! Processo #%d nao encontrado.\n", process_id);
                    continue;
					}
                    break;
        case 3: // Finalizar Processo
            printf("Insira o ID do Processo a ser finalizado: ");
            int terminate_pid;
            scanf("%d", &terminate_pid);

            int terminated = 0;
            for (i = 0; i < processes_idx; i++) {
                if (running_processes[i].process_id == terminate_pid) {
                    terminate_process(&physical_memory, &running_processes[i]);
                    terminated = 1;
                    break;
                }
            }

            if (!terminated) {
                printf("ERRO!!! Processo #%d nao encontrado.\n", terminate_pid);
            }
            else {
            	processes_idx--;
			}
            break;
        case 4: // Exibir Status da Memória Física
            display_physical_memory_status(&physical_memory);
            break;

        case 5: // Exibir Status da Memória Virtual
            printf("Insira o ID do Processo para exibir o status da memoria virtual: ");
            int display_pid;
            scanf("%d", &display_pid);

            int displayed = 0;
            for (i = 0; i < processes_idx; i++) {
                if (running_processes[i].process_id == display_pid) {
                    display_virtual_memory_status(&running_processes[i]);
                    displayed = 1;
                    break;
                }
            }

            if (!displayed) {
                printf("ERRO!!! Processo #%d nao encontrado.\n", display_pid);
            }
            break;
        default:
            printf("---\nOpcao Invalida!!! Tente Novamente apenas com valores de 1 a 6...\n");
            break;
        }
    }

    return 0;
}
