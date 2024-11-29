/*
SIMULADOR DE PAGINACAO
---
Integrantes:
Vitor Alves Pereira, 10410862
Eduardo Takashi Missaka, 10417877
Tiago Silveira Lopez, 10417600
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

/* Page: estrutura da pagina
   'id': identificador unico da pagina
   'frame_id': identificador unico do frame que a pagina esta alocada */
typedef struct
{
    int id;
    int frame_id;
} Page;

/* PageTable: estrutura da tabela de paginas
   'process_id': identificador unico do processo que esta utilizando a tabela
   'total_of_pages': numero total de paginas do processo
   'pages': ponteiro para as paginas do processo */
typedef struct
{
    int process_id;
    int total_of_pages;
    Page *pages;
} PageTable;

/* Frame: estrutura do frame
   'frame_id': identificador unico do frame
   'process_id': identificador unico do processo que esta utilizando o frame
   'page_id': identificador unico da pagina que esta alocada no frame
   'status': identificador se esta alocado na memoria principal */
typedef struct
{
    int frame_id;
    int process_id;
    int page_id;
    int status;
} Frame;

/* PhysicalMemory: estrutura da memoria fisica
   'total_of_frames': numero total de frames da memoria fisica
   'frames': ponteiro para os frames do processo */
typedef struct
{
    int total_of_frames;
    Frame *frames;
} PhysicalMemory;

/* VirtualMemory: estrutura da memoria virtual
   'process_id': identificador unico do processo
   'total_of_pages': numero total de paginas do processo
   'page_table': tabela de paginas do processo */
typedef struct
{
    int process_id;
    int total_of_pages;
    PageTable page_table;
} VirtualMemory;

typedef struct {
    int *frame_ids;
    int capacity;
    int front;
    int rear;
    int size;
} Queue;

/* log_operation: funcao auxiliar para gerar logs
   'message': mensagem de log */
void log_operation(const char *message)
{
    FILE *log_file = fopen("simulation.log", "a");
    if (log_file == NULL)
    {
        printf("ERRO!!! Nao foi possivel criar o arquivo de log.\n");
        exit(1);
    }
    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

/* init_queue: funcao para inicializacao da fila */
void init_queue(Queue *queue, int capacity) {
    queue->frame_ids = (int *)malloc(capacity * sizeof(int));
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
}

/* is_queue_empty: fila vazia?? */
bool is_queue_empty(Queue *queue) {
    return queue->size == 0;
}

/* is_queue_full: fila cheia?? */
bool is_queue_full(Queue *queue) {
    return queue->size == queue->capacity;
}

/* enqueue: funcao para adicionar a fila
            determinado frame */
void enqueue(Queue *queue, int frame_id) {
    if (is_queue_full(queue)) {
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->frame_ids[queue->rear] = frame_id;
    queue->size++;
}

/* dequeue: funcao para remover frame 
            no inicio da fila*/
int dequeue(Queue *queue) {
    if (is_queue_empty(queue)) {
        return -1;
    }
    int frame_id = queue->frame_ids[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return frame_id;
}

/* remove_page_from_queue: funcao para remover um frame 
            de um processo finalizado*/
void remove_page_from_queue(Queue *queue, int frame_id) {
    Queue temp_queue;
    init_queue(&temp_queue, queue->capacity);

    while (!is_queue_empty(queue)) {
        int current_page = dequeue(queue);
        if (current_page != frame_id) {
            enqueue(&temp_queue, current_page);
        }
    }

    while (!is_queue_empty(&temp_queue)) {
        enqueue(queue, dequeue(&temp_queue));
    }

}

/* init_process: funcao de inicializacao do processo
   'pid': identificador unico do processo
   'total_of_pages': numero total de paginas do processo */
VirtualMemory init_process(int pid, int total_of_pages)
{
    printf("Inicializando processo na memoria virtual...\n");
    VirtualMemory virtual_memory;
    virtual_memory.process_id = pid;
    virtual_memory.total_of_pages = total_of_pages;

    virtual_memory.page_table.process_id = pid;
    virtual_memory.page_table.total_of_pages = total_of_pages;

    printf("Alocando %lld bytes na memoria para as paginas do Processo #%d\n", total_of_pages * sizeof(Page), pid);
    virtual_memory.page_table.pages = (Page *)malloc(total_of_pages * sizeof(Page));
    
    // Validacao: erro no processo de alocacao
    if (virtual_memory.page_table.pages == NULL)
    {
        printf("ERRO!!! Nao foi possivel alocar espaco na memoria para o Processo #%d com %d paginas. Tente Novamente...\n", pid, total_of_pages);
        exit(1);
    }
    
    for (int i = 0; i < total_of_pages; i++)
    {
        virtual_memory.page_table.pages[i].id = i;
        virtual_memory.page_table.pages[i].frame_id = -1; // '-1' indica que o frame nao foi alocado na memoria fisica
    }

    char log_msg[100];
    sprintf(log_msg, "PROCESS_CREATED: Processo #%d com %d paginas inicializado.", pid, total_of_pages);
    log_operation(log_msg);

    return virtual_memory;

}

/* init_physical_memory: funcao de inicializacao da memoria fisica
   'physical_memory': estrutura da memoria fisica
   'total_of_frames': numero total de frames da memoria fisica */
void init_physical_memory(PhysicalMemory *physical_memory, int total_of_frames)
{
    physical_memory->total_of_frames = total_of_frames;
    printf("Alocando %lld bytes na memoria fisica para os frames...\n", total_of_frames * sizeof(Frame));
    physical_memory->frames = (Frame *)malloc(total_of_frames * sizeof(Frame));

    // Validacao: erro no processo de alocacao
    if (physical_memory->frames == NULL)
    {
        printf("ERRO!!! Nao foi possivel alocar espaco na memoria fisica...\n");
        exit(1);
    }
    
    for (int i = 0; i < total_of_frames; i++)
    {
        physical_memory->frames[i].frame_id = i;
        physical_memory->frames[i].process_id = -1; // '-1' indica que o frame i nao tem um processo
        physical_memory->frames[i].page_id = -1; // '-1' indica que o frame i nao tem paginas alocadas
        physical_memory->frames[i].status = -1; // '-1' indica que o frame i nao esta alocado na memoria fisica
    }
    
    char log_msg[100];
    sprintf(log_msg, "PHYSICAL_MEMORY_INITIALIZED: %d frames alocados.", total_of_frames);
    log_operation(log_msg);
}

/* handle_page_fault: funcao auxiliar para gerenciar page faults
   'physical_memory': memoria fisica
   'virtual_memory': memoria virtual
   'page_id': identificador unico da pagina
   'sleep_time': tempo de espera durante a busca na memoria secundaria */
void handle_page_fault(PhysicalMemory *physical_memory, VirtualMemory *virtual_memory, int page_id, int sleep_time, Queue *FIFO_queue)
{
    printf("PAGEFAULT001 - Pagina #%d do Processo #%d\n", page_id, virtual_memory->process_id);
    log_operation("PAGEFAULT001");

    sleep(sleep_time);

    int victim_frame;
    
    //Escolher frame
    if (!is_queue_full(FIFO_queue)) {
        for (int i = 0; i < physical_memory->total_of_frames; i++) {
            if (physical_memory->frames[i].status == -1) {
                victim_frame = i;
                break;
            }
        }
    } else { 
        // Caso contrario todos os frames estão ocupados, frame a ser substituido eh o primeiro da fila
        victim_frame = dequeue(FIFO_queue);
    }

    // Substituir pagina no frame escolhido aleatoriamente
    Frame *frame = &physical_memory->frames[victim_frame];

    // Caso o frame esteja ocupado, deve substituir pela pagina atual
    if (frame->process_id != -1)
    {
        VirtualMemory *victim_process = virtual_memory;
        victim_process->page_table.pages[frame->page_id].frame_id = -1; // Reiniciar valor para -1
        
        char log_msg[100];
        sprintf(log_msg, "Substituindo Pagina #%d do Processo #%d no Frame #%d", frame->page_id, frame->process_id, victim_frame);
        log_operation(log_msg);

    }
    //Atualizar dados do frame: 'process_id' e 'page_id'
    frame->process_id = virtual_memory->process_id;
    frame->page_id = page_id;
    //Atualizar tabela de paginas
    virtual_memory->page_table.pages[page_id].frame_id = victim_frame;
    
    //Adicionar na fila
    enqueue(FIFO_queue, victim_frame);
    
    printf("Pagina #%d alocada no Frame #%d\n", page_id, victim_frame);
    
    char log_msg[100];
    sprintf(log_msg, "PAGE_ALLOCATED: Pagina #%d do Processo #%d alocada no Frame #%d.", page_id, virtual_memory->process_id, victim_frame);
    log_operation(log_msg);
}

/* get_frame: funcao auxiliar para retornar frame alocado da página (-1 caso não esteja em nenhum frame)
   'virtual_memory': memoria virtual
   'page_id': identificador unico da pagina */
int get_frame(VirtualMemory *virtual_memory, int page_id)
{
    // Verifica se o page_id esta dentro do intervalo valido (obs: provavelmente nao utilizado, pois o check ja e feito em 'allocate_page')
    if (page_id < 0 || page_id >= virtual_memory->total_of_pages)
    {
        printf("ERRO!!! Pagina #%d esta fora do intervalo valido (0-%d).\n", 
               page_id, virtual_memory->total_of_pages - 1);
        return -1;
    }

    // Obter o frame_id associado a pagina
    int frame_id = virtual_memory->page_table.pages[page_id].frame_id;

    // Depuracao: mostra o estado atual da pagina
    printf("Debug: translate_address - Processo #%d, Pagina #%d, Frame ID = %d\n",
           virtual_memory->process_id, page_id, frame_id);

    return frame_id; // Retorna o frame_id (ou -1 se nao estiver mapeado)
}

/* allocate_page: funcao de alocacao na memoria fisica
   'physical_memory': memoria fisica
   'virtual_memory': memoria virtual
   'page_id': identificador unico da pagina
   'sleep_time': sleep */
void allocate_page(PhysicalMemory *physical_memory, VirtualMemory *virtual_memory, int page_id, int sleep_time, Queue *FIFO_queue, int page_size)
{
    // Verifica se o page_id e valido: deve estar entre 0 e ('virtual_memory->total_of_pages' - 1)
    if (page_id < 0 || page_id >= virtual_memory->total_of_pages)
    {
        printf("ERRO!!! ID de Pagina invalido: %d. Intervalo permitido: 0-%d.\n", 
               page_id, virtual_memory->total_of_pages - 1);
        return;
    }

    Page *page = &virtual_memory->page_table.pages[page_id]; 

    if (page->frame_id != -1) {
        printf("Pagina #%d do Processo #%d ja esta alocada no Frame #%d.\n", page_id, virtual_memory->process_id, page->frame_id);
        return;
    }
    
    // Verifica se a memoria fisica foi inicializada
    if (physical_memory->frames == NULL)
    {
        printf("ERRO!!! Memoria fisica nao inicializada.\n");
        return;
    }

    // Traduz o endereco logico (page_id) para o endereco fisico (frame_id)
    int frame_id = get_frame(virtual_memory, page_id);

    printf("Debug: allocate_page - Processo #%d, Pagina #%d, Frame ID retornado = %d\n",
           virtual_memory->process_id, page_id, frame_id);

    if (frame_id == -1)
    {
        printf("PAGE FAULT! Pagina #%d do Processo #%d nao esta mapeada para nenhum Frame.\n", 
               page_id, virtual_memory->process_id);
        
        handle_page_fault(physical_memory, virtual_memory, page_id, sleep_time, FIFO_queue);
        
        //Agora a pagina deve estar alocada, erro caso contrario
        frame_id = get_frame(virtual_memory, page_id);

        if (frame_id != -1) {
            printf("Pagina #%d do Processo #%d mapeada para Frame #%d apos Page Fault.\n", page_id, virtual_memory->process_id, frame_id);
            
            unsigned long int logical_address = page_id * page_size;
            printf("Endereco Logico da Pagina: %lu\n", logical_address);
            
            unsigned long int page_size_bytes = page_size * 1024;
			
			// Calcular o deslocamento
			unsigned long int offset = logical_address % page_size_bytes;
			
			// Calcular endereço físico
			unsigned long int physical_address = (frame_id * page_size_bytes) + offset;
			
            printf("Endereco Fisico da Pagina: %d\n", physical_address);
        }
            
        else {
            printf("ERRO!!! Pagina #%d do Processo #%d ainda nao esta mapeada apos Page Fault.\n",
                   page_id, virtual_memory->process_id);
               }
    }

    // Pagina mapeada, alocando no frame correspondente
    Frame *frame = &physical_memory->frames[frame_id];
    frame->process_id = virtual_memory->process_id;
    frame->page_id = page_id;
    frame->status = 1; // '1' determina que o frame agora esta ocupado

    // Atualizacao na tabela de paginas
    virtual_memory->page_table.pages[page_id].frame_id = frame_id;

    printf("Pagina #%d do Processo #%d alocada no Frame #%d.\n", page_id, virtual_memory->process_id, frame_id);

    sleep(sleep_time);
    
    char log_msg[100];
    sprintf(log_msg, "PAGE_ALLOCATED: Pagina #%d do Processo #%d alocada com sucesso no Frame #%d.", page_id, virtual_memory->process_id, frame_id);
    log_operation(log_msg);
}

/* terminate_process: funcao auxiliar para finalizar um processo
   'physical_memory': memoria fisica
   'virtual_memory': memoria virtual */
void terminate_process(PhysicalMemory *physical_memory, VirtualMemory *virtual_memory, Queue *FIFO_queue) {

    int pid = virtual_memory->process_id;
    printf("Finalizando Processo #%d...\n", pid);

    // Liberar os frames ocupados pelo processo na memoria fisica
    for (int i = 0; i < virtual_memory->total_of_pages; i++) {
        int frame_id = virtual_memory->page_table.pages[i].frame_id;
        if (frame_id != -1) {
            physical_memory->frames[frame_id].process_id = -1;
            physical_memory->frames[frame_id].page_id = -1;
            physical_memory->frames[frame_id].status = -1;
            
            remove_page_from_queue(FIFO_queue, frame_id);
        }
    }

    // Liberacao e reinicializacao variaveis da memoria virtual do processo
    free(virtual_memory->page_table.pages);
    virtual_memory->page_table.pages = NULL;
    virtual_memory->total_of_pages = 0;
    virtual_memory->process_id = -1;

    printf("Processo #%d finalizado com sucesso.\n", pid);
    
    char log_msg[100];
    sprintf(log_msg, "PROCESS_TERMINATED: Processo #%d exterminado.", pid);
    log_operation(log_msg);
}

/* display_physical_memory_status: funcao auxiliar para imprimir os frames da memoria fisica */
void display_physical_memory_status(PhysicalMemory *physical_memory) {
    printf("\n---\nMEMORIA FISICA\n---\n");
    for (int i = 0; i < physical_memory->total_of_frames; i++) {
        Frame *frame = &physical_memory->frames[i];
        printf("Frame #%d: ", frame->frame_id);
        if (frame->status == -1) {
            printf("LIVRE\n");
        } else {
            printf("OCUPADO (Processo #%d, Pagina #%d)\n", frame->process_id, frame->page_id);
        }
    }
}

/* display_virtual_memory_status: funcao auxiliar para imprimir as paginas da memoria virtual */
void display_virtual_memory_status(VirtualMemory *virtual_memory) {
	int i;
    if (virtual_memory->process_id == -1 || virtual_memory->page_table.pages == NULL) {
        printf("Nenhum processo carregado na memoria virtual.\n");
        return;
    }

    printf("\n---\nMEMORIA VIRTUAL DO PROCESSO #%d\n---\n", virtual_memory->process_id);
    for (i = 0; i < virtual_memory->total_of_pages; i++) {
        int frame_id = virtual_memory->page_table.pages[i].frame_id;
        printf("Pagina #%d: ", virtual_memory->page_table.pages[i].id);
        if (frame_id == -1) {
            printf("NAO MAPEADA\n");
        } else {
            printf("Mapeada para Frame #%d\n", frame_id);
        }
    }
}

/* validate_configuration: funcao auxiliar para validar as informacoes do arquivo de configuracoes
   'total_of_frames': numero total de frames
   'total_of_pages': numero total de paginas
   'total_of_processes': numero total de processos
   'sleep_time': delay padrao */
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
        printf("ERRO: Numero de processos invalido! Deve ser maior que zero.\n");
        return 0;
    }

    if (sleep_time < 0) {
        printf("ERRO: Tempo de espera invalido! Deve ser zero ou maior.\n");
        return 0; 
    }

    return 1;
}

int main()
{

    printf("\nSIMULADOR DE PAGINACAO\n---\n");
    printf("Lendo Arquivo de Config...\n");
    sleep(1);

    FILE *file_ptr = fopen("../../docs/config.txt", "r");

    while (file_ptr == NULL)
    {
        printf("---\nArquivo de Configuracao nao encontrado no caminho \"../../docs/config.txt\"!!! Tente novamente inserindo o caminho manualmente...\n");
        char new_path[100];
        printf("Insira o caminho: ");
        scanf("%s", new_path);
        file_ptr = fopen(new_path, "r");
    }

    printf("Arquivo de Config Aberto com Sucesso!\n");
    
    int value;
    char buffer[100];
    char key[100];
    int processes_idx = 0;
    int total_of_frames = 0;
    int page_size = 0;
    int total_of_processes = 0;
    int sleep_time = 0;

    while (fgets(buffer, 100, file_ptr))
    {
        if (sscanf(buffer, "%s = %d", key, &value))
        {
            if (strcmp(key, "total_frames") == 0)
                total_of_frames = value; //TOTAL DE FRAMES NA MEMÓRIA FÍSICA
            else if (strcmp(key, "page_size") == 0)
                page_size = value;
            else if (strcmp(key, "processes") == 0)
                total_of_processes = value; //TOTAL DE PROCESSOS
            else if (strcmp(key, "sleep_time") == 0)
                sleep_time = value; //TEMPO DE ESPERA PARA ACESSAR MEMÓRIA SECUNDÁRIA
        }
    }
    
    fclose(file_ptr);
    
    if (!validate_configuration(total_of_frames, page_size, total_of_processes, sleep_time)) {
        printf("---\nERRO: Valores invalidos no arquivo de configuracao! Tente Novamente...\n");
        exit(1);
    }
    
    printf("Inicializando Memoria Fisica...\n");
    PhysicalMemory physical_memory;
    Queue queue;
    init_physical_memory(&physical_memory, total_of_frames);
    init_queue(&queue, total_of_frames);
    sleep(1);

    int menu_option;
    int found;
    int terminated = 0;
    int displayed = 0;
    int pid, total_pages = 0;
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

        case 1:

            if (processes_idx >= total_of_processes)
            {
                printf("---\nERRO!!! Limite de processos atingido.\n");
                break;
            }

            pid, total_pages = 0;
            printf("---\nInsira o ID do Processo e o Total de Paginas (pid total): ");
            scanf("%d %d", &pid, &total_pages);
            
            while (total_pages <= 0) {
            	printf("---\nERRO: O numero total de paginas deve ser maior que zero!\n");
                printf("\nInsira o ID do Processo e o Total de Paginas (pid total): ");
                scanf("%d %d", &pid, &total_pages);
			}
			
            running_processes[processes_idx] = init_process(pid, total_pages);
            printf("Processo #%d criado com %d paginas.\n", pid, total_pages);
            processes_idx++;
            
            break;

        case 2:
            
            printf("---\nInsira o ID do Processo e o ID da Pagina (process_id page_id): ");
            int process_id, page_id;
            scanf("%d %d", &process_id, &page_id);

            found = 0;
            
            for (int i = 0; i < processes_idx; i++)
            {
                if (running_processes[i].process_id == process_id)
                {
                    allocate_page(&physical_memory, &running_processes[i], page_id, sleep_time, &queue, page_size);
                    found = 1;
                }
            }
            
            if (!found)
			    printf("---\nERRO: Processo #%d nao encontrado.\n", process_id);
			
            break;

        case 3:

            printf("---\nInsira o ID do Processo a ser finalizado: ");
            int terminate_pid;
            scanf("%d", &terminate_pid);

            for (int i = 0; i < processes_idx; i++) {
                if (running_processes[i].process_id == terminate_pid) {
                    terminate_process(&physical_memory, &running_processes[i], &queue);
                    terminated = 1;
                    break;
                }
            }

            if (!terminated)
                printf("---\nERRO: Processo #%d nao encontrado.\n", terminate_pid);
            else
            	processes_idx--;
			
            break;

        case 4:

            display_physical_memory_status(&physical_memory);
            break;

        case 5:
            printf("---\nInsira o ID do Processo para exibir o status da memoria virtual: ");
            int display_pid;
            scanf("%d", &display_pid);

            for (int i = 0; i < processes_idx; i++) {
                if (running_processes[i].process_id == display_pid) {
                    display_virtual_memory_status(&running_processes[i]);
                    displayed = 1;
                    break;
                }
            }

            if (!displayed)
                printf("ERRO: Processo #%d nao encontrado.\n", display_pid);
            
            break;
        
        default:
            printf("---\nOpcao Invalida!!! Tente Novamente apenas com valores de 1 a 6...\n");
            break;
        
        }
    }

    return 0;
}
