# --- Toolchain ---
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
ASM = nasm

# --- Flags ---
# -mno-sse -mno-mmx: Previne o crash de "Invalid Opcode" (setup de segurança)
CFLAGS = -m32 -ffreestanding -g -O2 -Wall -Wextra -I src/include -fno-pie -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mno-80387
LDFLAGS = -m elf_i386 -T linker.ld

# --- Arquivos ---
# Pega todos os .c em kernel e drivers (incluindo o ata.c novo)
C_SOURCES = $(wildcard src/kernel/*.c) $(wildcard src/drivers/*.c)
ASM_SOURCES = $(wildcard src/arch/x86/*.asm)

# --- Objetos ---
C_OBJECTS = $(patsubst src/%.c, build/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst src/arch/x86/%.asm, build/arch/%.o, $(ASM_SOURCES))

KERNEL_BIN = build/kernel.bin

# --- Regras ---

# O padrão agora é criar o Kernel E o Disco
all: $(KERNEL_BIN) disk.img

# Regra para criar o disco de 10MB (apenas se não existir)
disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=10

$(KERNEL_BIN): $(ASM_OBJECTS) $(C_OBJECTS)
	@mkdir -p build
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(ASM_OBJECTS) $(C_OBJECTS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/arch/%.o: src/arch/x86/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf32 $< -o $@

# O 'run' agora depende do 'disk.img'
run: $(KERNEL_BIN) disk.img
	qemu-system-i386 -kernel $(KERNEL_BIN) -hda disk.img -display cocoa

clean:
	rm -rf build disk.img