// apps/calc.c
// Calculadora Completa (0-9, +, -, *, /, =, C)

// --- 1. Protótipos ---
void sys_exit();
int sys_create_window(char* title, int x, int y, int w, int h);
void sys_create_button(int win, int id, char* text, int x, int y, int w, int h);
int sys_wait_event(int win);
void sys_update_button(int win, int id, char* text);
void itoa(int n, char* buffer);

// --- 2. Entry Point ---
void main() {
    // Aumentei um pouco a altura para caber tudo (160x220)
    int win = sys_create_window("Calc", 100, 100, 160, 220);
    
    // Visor (ID 999) - Topo
    sys_create_button(win, 999, "0", 10, 10, 140, 30); 

    // --- LINHA 1 (7, 8, 9, /) ---
    sys_create_button(win, 7, "7", 10, 50, 30, 30);
    sys_create_button(win, 8, "8", 45, 50, 30, 30);
    sys_create_button(win, 9, "9", 80, 50, 30, 30);
    sys_create_button(win, 23, "/", 115, 50, 30, 30); // ID 23 = Divisão

    // --- LINHA 2 (4, 5, 6, *) ---
    sys_create_button(win, 4, "4", 10, 85, 30, 30);
    sys_create_button(win, 5, "5", 45, 85, 30, 30);
    sys_create_button(win, 6, "6", 80, 85, 30, 30);
    sys_create_button(win, 22, "*", 115, 85, 30, 30); // ID 22 = Multiplicação

    // --- LINHA 3 (1, 2, 3, -) ---
    sys_create_button(win, 1, "1", 10, 120, 30, 30);
    sys_create_button(win, 2, "2", 45, 120, 30, 30);
    sys_create_button(win, 3, "3", 80, 120, 30, 30);
    sys_create_button(win, 21, "-", 115, 120, 30, 30); // ID 21 = Subtração

    // --- LINHA 4 (C, 0, =, +) ---
    sys_create_button(win, 30, "C", 10, 155, 30, 30); // ID 30 = Clear
    sys_create_button(win, 0, "0", 45, 155, 30, 30);
    sys_create_button(win, 40, "=", 80, 155, 30, 30); // ID 40 = Igual
    sys_create_button(win, 20, "+", 115, 155, 30, 30); // ID 20 = Soma

    int acumulador = 0;   // Valor atual na tela
    int valor_salvo = 0;  // Valor guardado antes da operação
    int operacao = 0;     // 0=Nada, 1=+, 2=-, 3=*, 4=/
    char buf[32];

    while(1) {
        int id = sys_wait_event(win);

        // --- NÚMEROS (0 a 9) ---
        if (id >= 0 && id <= 9) {
            acumulador = acumulador * 10 + id;
            itoa(acumulador, buf);
            sys_update_button(win, 999, buf);
        }
        
        // --- OPERAÇÕES ---
        else if (id >= 20 && id <= 23) {
            valor_salvo = acumulador;
            acumulador = 0;
            
            if (id == 20) { operacao = 1; sys_update_button(win, 999, "+"); }
            if (id == 21) { operacao = 2; sys_update_button(win, 999, "-"); }
            if (id == 22) { operacao = 3; sys_update_button(win, 999, "*"); }
            if (id == 23) { operacao = 4; sys_update_button(win, 999, "/"); }
        }

        // --- IGUAL (=) ---
        else if (id == 40) {
            if (operacao == 1) acumulador = valor_salvo + acumulador;
            if (operacao == 2) acumulador = valor_salvo - acumulador;
            if (operacao == 3) acumulador = valor_salvo * acumulador;
            if (operacao == 4) {
                if (acumulador != 0) acumulador = valor_salvo / acumulador;
                else acumulador = 0; // Proteção contra divisão por zero
            }
            
            itoa(acumulador, buf);
            sys_update_button(win, 999, buf);
            operacao = 0; // Reseta operação
        }

        // --- CLEAR (C) ---
        else if (id == 30) {
            acumulador = 0;
            valor_salvo = 0;
            operacao = 0;
            sys_update_button(win, 999, "0");
        }
    }
    sys_exit();
}

// --- 3. Implementação das Syscalls (Assembly Manual Seguro) ---

void sys_exit() { 
    __asm__ volatile("int $0x80" :: "a"(0)); 
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
    int sign = 0;
    if (n == 0) { buffer[0] = '0'; buffer[1] = 0; return; }
    if (n < 0) { sign = 1; n = -n; } // Suporte a números negativos

    while (n > 0) { buffer[i++] = (n % 10) + '0'; n /= 10; }
    if (sign) buffer[i++] = '-';
    
    buffer[i] = 0;
    for(int j=0; j<i/2; j++) { char t=buffer[j]; buffer[j]=buffer[i-1-j]; buffer[i-1-j]=t; }
}