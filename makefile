# --- Toolchain ---
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
ASM = nasm

# --- Flags ---
# -mno-sse -mno-mmx: Previne o crash de "Invalid Opcode" (setup de segurança)
CFLAGS = -m32 -ffreestanding -g -O0 -Wall -Wextra -I src/include -fno-pie -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mno-80387
LDFLAGS = -m elf_i386 -T linker.ld

# --- Arquivos ---
# Pega todos os .c em kernel e drivers (incluindo o ata.c novo)
C_SOURCES = $(wildcard src/kernel/*.c) $(wildcard src/drivers/*.c)
ASM_SOURCES = $(wildcard src/arch/x86/*.asm)
APP_SOURCES = $(wildcard apps/*.c)
APP_OBJS = $(APP_SOURCES:.c=.o)
APP_BINS = $(patsubst apps/%.c, files/%.bin, $(APP_SOURCES))

# --- Objetos ---
C_OBJECTS = $(patsubst src/%.c, build/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst src/arch/x86/%.asm, build/arch/%.o, $(ASM_SOURCES))

KERNEL_BIN = build/kernel.bin
PACKER = tools/packer



# --- Regras ---

$(PACKER): tools/packer.c
	gcc tools/packer.c -o $(PACKER)

apps/%.o: apps/%.c
	x86_64-elf-gcc -m32 -ffreestanding -fno-pie -c $< -o $@

files/%.bin: apps/%.o
	mkdir -p files
	x86_64-elf-ld -m elf_i386 -Ttext 0x400000 --oformat binary $< -o $@

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
# O run agora depende de TODOS os binários gerados automaticamente
run: $(KERNEL_BIN) disk.img $(PACKER) $(APP_BINS)
# Loop Shell: Para cada binário gerado, injeta no disco
	@for bin in $(APP_BINS); do \
		PROG_NAME=$$(basename $$bin .bin); \
		echo "Injetando $$PROG_NAME..."; \
		./$(PACKER) disk.img $$bin $$PROG_NAME; \
	done

# Roda o QEMU
	qemu-system-i386 -kernel $(KERNEL_BIN) -hda disk.img -vga cirrus \
    -no-reboot -no-shutdown -serial stdio
clean:
	rm -rf build/* files/* apps/*.o tools/packer disk.img