#include "../include/shell.h"
#include "../include/utils.h"
#include "../include/api.h" 

// --- Funções Auxiliares (Iguais a antes) ---

char* get_arg(char* input) {
    int i = 0;
    while(input[i] != ' ' && input[i] != '\0') i++;
    if(input[i] == '\0') return ""; 
    return &input[i+1];
}

int str_starts_with(char* input, char* prefix) {
    int i = 0;
    while(prefix[i] != '\0') {
        if(input[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
}

void get_content_input(char* buffer) {
    int index = 0;
    os_print("Digite o texto e aperte ENTER:\n>> "); // Tirei cores por enquanto pra simplificar no grafico
    
    while(1) {
        char c = os_wait_for_key();
        
        if(c == '\n') {
            buffer[index] = '\0';
            os_print("\n");
            return;
        } else if (c == '\b') {
            if(index > 0) { 
                index--; 
                buffer[index]=0; 
                os_print("\b"); // O driver VGA gráfico lida com isso apagando o quadrado
            }
        } else {
            if(index < 500) {
                char temp[2] = {c, 0};
                os_print(temp);
                buffer[index] = c;
                index++;
            }
        }
    }
}

// --- Comandos ---

void cmd_ajuda() {
    os_print("--- Comandos Graficos ---\n");
    os_print("  janela        - Cria uma janela de teste\n"); // <--- NOVO
    os_print("  limpar        - Limpa e redesenha o shell\n");
    os_print("--- Arquivos ---\n");
    os_print("  ls            - Lista arquivos\n");
    os_print("  criar [nome]  - Cria arquivo\n");
    os_print("  cat [nome]    - Le arquivo\n");
    os_print("  rm [nome]     - Apaga arquivo\n");
    os_print("--- Sistema ---\n");
    os_print("  reboot        - Reinicia\n");
}

void shell_init() {
    // Ao iniciar, desenha a janela principal do Shell
    // Título, X=0, Y=0, Largura=320, Altura=200, Cor=1 (Azul)
    os_create_window("Shell do MyOS", 10, 10, 280, 160, 0);
    
    // Pula umas linhas para não escrever em cima da barra de título
    os_print("\n\n"); 
    os_print("Sistema Grafico Iniciado.\n");
}

// --- O CÉREBRO DO SHELL ---
void shell_execute(char* input) {
    if (strlen(input) == 0) return;
    os_print("\n");

    if (strcmp(input, "ajuda") == 0) {
        cmd_ajuda();
    }
    // --- COMANDO NOVO: JANELA ---
    else if (strcmp(input, "janela") == 0) {
        // Cria uma janela verde (cor 2) no meio da tela para testar a API
        os_create_window("Janela Teste", 50, 50, 150, 100, 2);
    }
    // --- COMANDO ATUALIZADO: LIMPAR ---
    else if (strcmp(input, "limpar") == 0) {
        // No modo gráfico, limpar a tela apaga a janela do shell!
        // Então temos que limpar E redesenhar a janela.
        os_clear_screen();
        os_create_window("Shell do MyOS", 0, 0, 320, 200, 1);
        os_print("\n\n");
    }
    else if (strcmp(input, "ls") == 0) {
        os_file_list();
    }
    else if (strcmp(input, "reboot") == 0) {
        os_reboot();
    }
    else if (str_starts_with(input, "cat ")) {
        char* filename = get_arg(input);
        char buffer[512];
        if(os_file_read(filename, buffer)) {
            os_print("--- Conteudo ---\n");
            os_print(buffer);
            os_print("\n----------------\n");
        } else {
            os_print("Erro: Arquivo nao encontrado.\n");
        }
    }
    else if (str_starts_with(input, "rm ")) {
        char* filename = get_arg(input);
        if(os_file_delete(filename)) {
            os_print("Arquivo apagado.\n");
        } else {
            os_print("Erro ao apagar.\n");
        }
    }
    else if (str_starts_with(input, "criar ")) {
        char* filename = get_arg(input);
        if(strlen(filename) > 0) {
            char content_buffer[512];
            get_content_input(content_buffer);
            if(os_file_create(filename, content_buffer)) {
                os_print("Sucesso!\n");
            } else {
                os_print("Erro ao criar.\n");
            }
        }
    }
    else {
        os_print("Comando desconhecido.\n");
    }
}