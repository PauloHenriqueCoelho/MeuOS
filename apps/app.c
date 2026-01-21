// apps/simple.c

// 1. Cabeçalho Obrigatório (Entrada do Programa)
__asm__ (
    ".section .text\n"
    ".global _start\n"
    "_start:\n"
    "call main\n"      // Chama o C
    "jmp .\n"          // Se voltar, trava aqui (loop infinito)
);

// 2. Syscalls Manuais (Sem depedências)
void sys_print(char* msg) {
    // EAX=1 -> Print
    __asm__ volatile("int $0x80" : : "a"(1), "b"(msg));
}

int sys_create_window(char* title, int x, int y, int w, int h) {
    int ret; 
    int pos = (x << 16) | (y & 0xFFFF);  // Empacota X e Y
    int size = (w << 16) | (h & 0xFFFF); // Empacota W e H
    
    // EAX=12 -> Create Window
    __asm__ volatile(
        "int $0x80" 
        : "=a"(ret) 
        : "a"(12), "b"(title), "c"(pos), "d"(size)
        : "memory"
    ); 
    return ret;
}

// 3. Programa Principal
void main() {
    sys_print("\n[SIMPLE] 1. Programa iniciado.\n");

    // Tenta criar uma janela verde (ou cinza, dependendo do kernel) em 50,50
    int id = sys_create_window("Teste Simples", 50, 50, 200, 100);

    if (id >= 0) {
        sys_print("[SIMPLE] 2. Sucesso! Janela criada com ID: ");
        // Hack para mostrar ID (0-9)
        char c[2] = { id + '0', 0 }; 
        sys_print(c);
        sys_print("\n");
        sys_print("[SIMPLE] 3. Entrando em loop infinito...\n");
    } else {
        sys_print("[SIMPLE] ERRO: Falha ao criar janela (Retornou -1).\n");
    }

    // Mantém o programa rodando para a janela não sumir
    while(1) {
        // Nada, apenas exista.
    }
}