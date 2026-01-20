# --- Toolchain ---
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
ASM = nasm

# --- Flags (CORREÇÃO 3: Flags de segurança desativadas) ---
# -fno-pie: Não criar código independente de posição (essencial para kernel simples)
# -fno-stack-protector: Não usar "canary" na stack (evita crash de pilha)
CFLAGS = -m32 -ffreestanding -g -O2 -Wall -Wextra -I src/include -fno-pie -fno-stack-protector
LDFLAGS = -m elf_i386 -T linker.ld

# --- Arquivos ---
C_SOURCES = $(wildcard src/kernel/*.c) $(wildcard src/drivers/*.c)
ASM_SOURCES = $(wildcard src/arch/x86/*.asm)

# --- Objetos ---
C_OBJECTS = $(patsubst src/%.c, build/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst src/arch/x86/%.asm, build/arch/%.o, $(ASM_SOURCES))

KERNEL_BIN = build/kernel.bin

# --- Regras ---
all: $(KERNEL_BIN)

$(KERNEL_BIN): $(ASM_OBJECTS) $(C_OBJECTS)
	@mkdir -p build
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(ASM_OBJECTS) $(C_OBJECTS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/arch/%.o: src/arch/x86/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf32 $< -o $@

run: $(KERNEL_BIN)
	qemu-system-i386 -kernel build/kernel.bin \
    -d int,cpu_reset,guest_errors \
    -no-reboot -no-shutdown -serial stdio

clean:
	rm -rf build