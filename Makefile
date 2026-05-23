all:
	nasm -f elf32 boot/multiboot2.asm -o multiboot.o
	nasm -f elf32 boot/boot.asm -o boot.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/kernel.cpp \
	-o kernel.o

	x86_64-elf-g++ \
	-m32 \
	-nostdlib \
	-Wl,-m,elf_i386 \
	-T linker.ld \
	-o kernel.elf \
	boot.o multiboot.o kernel.o

run:
	qemu-system-x86_64 -cdrom neuralkernel.iso