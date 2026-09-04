CC = x86_64-elf-gcc
LD = x86_64-elf-ld
AS = nasm
GRUB = i686-elf-grub-mkrescue
QEMU = qemu-system-x86_64

CFLAGS = -m32 -O2 -ffreestanding -fno-pie -fno-stack-protector -nostdlib \
         -fno-tree-loop-distribute-patterns -fno-delete-null-pointer-checks \
         -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4 -mno-avx \
         -Isrc -Isrc/cpu -Isrc/io -Isrc/memory -Isrc/shell -Isrc/tasks -Isrc/utils -Isrc/tests
LDFLAGS = -m elf_i386 -T linker.ld

SOURCES_C := $(wildcard src/*.c src/*/*.c)
SOURCES_ASM := $(wildcard src/*.asm src/*/*.asm)

OBJECTS := $(SOURCES_C:.c=.o) \
           $(SOURCES_ASM:.asm=_asm.o)

all: xenonos.iso

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/%_asm.o: src/%.asm
	$(AS) -f elf32 $< -o $@

xenonos.bin: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

xenonos.iso: xenonos.bin grub.cfg
	mkdir -p iso/boot/grub
	cp xenonos.bin iso/boot/xenonos.bin
	cp grub.cfg iso/boot/grub/grub.cfg
	$(GRUB) -o $@ iso

run: xenonos.iso
	$(QEMU) -cdrom xenonos.iso

clean:
	rm -rf src/*.o src/*/*.o xenonos.bin xenonos.iso iso