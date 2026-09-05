CC = x86_64-elf-gcc
LD = x86_64-elf-ld
AS = nasm
GRUB = i686-elf-grub-mkrescue
QEMU = qemu-system-x86_64

GUI_WIDTH ?= 1920
GUI_HEIGHT ?= 1200
# to trigger a recompilation
GUI_RESOLUTION_STAMP := .gui_resolution-$(GUI_WIDTH)x$(GUI_HEIGHT)

CFLAGS = -m32 -O2 -ffreestanding -fno-pie -fno-stack-protector -nostdlib \
         -fno-tree-loop-distribute-patterns -fno-delete-null-pointer-checks \
         -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4 -mno-avx \
         -DGUI_WIDTH=$(GUI_WIDTH) -DGUI_HEIGHT=$(GUI_HEIGHT) \
         -Isrc -Isrc/cpu -Isrc/io -Isrc/memory -Isrc/shell -Isrc/tasks -Isrc/utils -Isrc/tests -Isrc/gui
LDFLAGS = -m elf_i386 -T linker.ld

SOURCES_C := $(wildcard src/*.c src/*/*.c)
SOURCES_ASM := $(wildcard src/*.asm src/*/*.asm)

OBJECTS := $(SOURCES_C:.c=.o) \
           $(SOURCES_ASM:.asm=_asm.o)

all: xenonos.iso

src/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

src/shell/commands.o: $(GUI_RESOLUTION_STAMP)

$(GUI_RESOLUTION_STAMP):
	rm -f .gui_resolution-*
	touch $@

src/%_asm.o: src/%.asm
	$(AS) -f elf32 $< -o $@

xenonos.bin: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

xenonos.iso: xenonos.bin grub.cfg
	mkdir -p iso/boot/grub
	cp xenonos.bin iso/boot/xenonos.bin
	cp grub.cfg iso/boot/grub/grub.cfg
	$(GRUB) -o $@ iso

run: xenonos-qemu.iso
	$(QEMU) -cdrom xenonos-qemu.iso -serial stdio

# QEMU build: same sources, but boot.asm skips the framebuffer request tag
# (unsafe in QEMU/SeaBIOS - see src/boot.asm). xenonos.iso is the real-
# hardware build and is the default target.
OBJECTS_QEMU := $(filter-out src/boot_asm.o,$(OBJECTS)) src/boot_qemu_asm.o

src/boot_qemu_asm.o: src/boot.asm
	$(AS) -f elf32 -D QEMU_BUILD $< -o $@

xenonos-qemu.bin: $(OBJECTS_QEMU) linker.ld
	$(LD) $(LDFLAGS) $(OBJECTS_QEMU) -o $@

xenonos-qemu.iso: xenonos-qemu.bin grub.cfg
	mkdir -p iso-qemu/boot/grub
	cp xenonos-qemu.bin iso-qemu/boot/xenonos.bin
	cp grub.cfg iso-qemu/boot/grub/grub.cfg
	$(GRUB) -o $@ iso-qemu

clean:
	rm -rf src/*.o src/*/*.o xenonos.bin xenonos.iso iso xenonos-qemu.bin xenonos-qemu.iso iso-qemu .gui_resolution-*