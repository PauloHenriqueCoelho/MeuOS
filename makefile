# --- Toolchain ---
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
ASM = nasm

# --- Flags ---
CFLAGS = -m32 -ffreestanding -g -O0 -Wall -Wextra -I src/include -fno-pie -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mno-80387
LDFLAGS = -m elf_i386 -T linker.ld

# --- Arquivos ---
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
ISO = myos.iso

# --- Regras Principais ---

all: $(ISO) disk.img $(PACKER) $(APP_BINS)

# --- Criação da ISO com GRUB ---
$(ISO): $(KERNEL_BIN)
	mkdir -p isodir/boot/grub
	cp $(KERNEL_BIN) isodir/boot/myos.bin
	echo 'menuentry "MeuOS" {' > isodir/boot/grub/grub.cfg
	echo '  multiboot /boot/myos.bin' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	i686-elf-grub-mkrescue -o $(ISO) isodir

# --- Ferramentas ---
$(PACKER): tools/packer.c
	gcc tools/packer.c -o $(PACKER)

# --- Apps do Usuário ---
apps/%.o: apps/%.c
	x86_64-elf-gcc -m32 -ffreestanding -fno-pie -c $< -o $@

files/%.bin: apps/%.o
	mkdir -p files
	x86_64-elf-ld -m elf_i386 -T app_linker.ld --oformat binary $< -o $@

# --- Disco ---
disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=10

# --- Kernel ---
$(KERNEL_BIN): $(ASM_OBJECTS) $(C_OBJECTS)
	@mkdir -p build
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(ASM_OBJECTS) $(C_OBJECTS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/arch/%.o: src/arch/x86/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf32 $< -o $@

# --- RUN ---
run: $(ISO) disk.img $(PACKER) $(APP_BINS)
	@for bin in $(APP_BINS); do \
		PROG_NAME=$$(basename $$bin); \
		echo "Injetando $$PROG_NAME..."; \
		./$(PACKER) disk.img $$bin $$PROG_NAME; \
	done

	qemu-system-i386 -cdrom $(ISO) -hda disk.img -vga std \
	-d int,guest_errors,cpu_reset -D qemu.log \
	-no-reboot -no-shutdown -serial stdio

clean:
	rm -rf build/* files/* apps/*.o tools/packer disk.img isodir *.iso qemu.log