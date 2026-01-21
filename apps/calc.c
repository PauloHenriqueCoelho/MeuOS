// apps/calc.c
// Compilar como binário "flat"

// --- 1. Protótipos (Para o main saber que essas funções existem) ---
void sys_exit();
void sys_print(char* s);
int sys_create_window(char* title, int x, int y, int w, int h);
void sys_create_button(int win, int id, char* text, int x, int y, int w, int h);
int sys_wait_event(int win);
void sys_update_button(int win, int id, char* text);
void itoa(int n, char* buffer);

// --- 2. Entry Point (O main DEVE ser a primeira função aqui!) ---
void main() {
    int win = sys_create_window("Calculadora", 100, 100, 160, 200);
    
    // Visor (ID 999)
    sys_create_button(win, 999, "0", 10, 10, 140, 30); 

    // Botões
    sys_create_button(win, 1, "1", 10, 50, 40, 40);
    sys_create_button(win, 2, "2", 60, 50, 40, 40);
    sys_create_button(win, 3, "+", 110, 50, 40, 40);
    sys_create_button(win, 4, "=", 110, 100, 40, 40);

    int acumulador = 0;
    int operacao = 0; 
    char buf[32];

    while(1) {
        int id = sys_wait_event(win);

        if (id == 1) { 
            acumulador = acumulador * 10 + 1;
            itoa(acumulador, buf);
            sys_update_button(win, 999, buf);
        }
        else if (id == 2) { 
            acumulador = acumulador * 10 + 2;
            itoa(acumulador, buf);
            sys_update_button(win, 999, buf);
        }
        else if (id == 3) { 
            operacao = 1; 
            acumulador = 0;
            sys_update_button(win, 999, "+");
        }
        else if (id == 4) { 
            if (operacao == 1) {
                acumulador = acumulador + 2; 
                itoa(acumulador, buf);
                sys_update_button(win, 999, buf);
                operacao = 0; 
            }
        }
    }
    sys_exit();
}

// --- 3. Implementação das Syscalls (Ficam embaixo) ---

void sys_exit() { 
    __asm__ volatile("int $0x80" :: "a"(0)); 
}

void sys_print(char* s) { 
    __asm__ volatile("int $0x80" :: "a"(1), "b"(s)); 
}

int sys_create_window(char* title, int x, int y, int w, int h) {
    int ret; 
    int pos = (x << 16) | (y & 0xFFFF);
    int size = (w << 16) | (h & 0xFFFF);
    
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret) 
        : "a"(12), "b"(title), "c"(pos), "d"(size)
        : "memory"
    ); 
    return ret;
}

void sys_create_button(int win, int id, char* text, int x, int y, int w, int h) {
    int pos = (x << 16) | (y & 0xFFFF);
    
    // Passagem manual via stack para evitar "impossible constraints"
    __asm__ volatile(
        "pushl %%ebp; "
        "pushl %%edi; "
        "pushl %%esi; "
        "pushl %%ebx; "

        "movl %1, %%ebx; "
        "movl %2, %%ecx; "
        "movl %3, %%edx; "
        "movl %4, %%esi; "
        "movl %5, %%edi; "
        "movl %6, %%ebp; "
        "movl $10, %%eax; "
        
        "int $0x80; "
        
        "popl %%ebx; "
        "popl %%esi; "
        "popl %%edi; "
        "popl %%ebp; "
        : 
        : "i"(10), "m"(win), "m"(pos), "m"(text), "m"(id), "m"(w), "m"(h)
        : "eax", "ecx", "edx", "memory", "cc"
    );
}

int sys_wait_event(int win) {
    int ret; 
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret) 
        : "a"(11), "b"(win)
        : "memory"
    ); 
    return ret;
}

void sys_update_button(int win, int id, char* text) {
    __asm__ volatile(
        "int $0x80"
        : 
        : "a"(13), "b"(win), "c"(id), "d"(text)
        : "memory"
    );
}

void itoa(int n, char* buffer) {
    int i = 0; 
    if (n == 0) { buffer[0] = '0'; buffer[1] = 0; return; }
    while (n > 0) { buffer[i++] = (n % 10) + '0'; n /= 10; }
    buffer[i] = 0;
    for(int j=0; j<i/2; j++) { char t=buffer[j]; buffer[j]=buffer[i-1-j]; buffer[i-1-j]=t; }
}