all:
	nasm -f elf32 boot/multiboot2.asm -o multiboot.o
	nasm -f elf32 boot/boot.asm -o boot.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/kernel.cpp -o kernel.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/terminal.cpp -o terminal.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/printk.cpp -o printk.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/gdt.cpp -o gdt.o


	nasm -f elf32 kernel/gdt.asm -o gdtasm.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/idt.cpp -o idt.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/isr.cpp -o isr.o


	nasm -f elf32 kernel/idt.asm -o idtasm.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/pic.cpp -o pic.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/pit.cpp -o pit.o


	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/irq.cpp -o irq.o


	nasm -f elf32 kernel/irq.asm -o irqasm.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/keyboard.cpp -o keyboard.o

	nasm -f elf32 kernel/keyboard.asm -o keyboardasm.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/shell.cpp -o shell.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/telemetry.cpp -o telemetry.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/pmm.cpp -o pmm.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/paging.cpp -o paging.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/heap.cpp -o heap.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/task.cpp -o task.o

	x86_64-elf-g++ \
	-m32 \
	-ffreestanding \
	-fno-exceptions \
	-fno-rtti \
	-c kernel/scheduler.cpp -o scheduler.o

	x86_64-elf-g++ \
	-m32 \
	-nostdlib \
	-Wl,-m,elf_i386 \
	-T linker.ld \
	-o kernel.elf \
	boot.o \
	multiboot.o \
	kernel.o \
	terminal.o \
	printk.o \
	gdt.o \
	gdtasm.o \
	idt.o \
	idtasm.o \
	isr.o \
	pic.o \
	pit.o \
	irq.o \
	irqasm.o \
	keyboard.o \
	keyboardasm.o \
	shell.o \
	telemetry.o \
	pmm.o \
	paging.o \
	heap.o \
	task.o \
	scheduler.o



iso: all
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/kernel.elf
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o neuralkernel.iso iso


run: iso
	qemu-system-x86_64 -cdrom neuralkernel.iso


clean:
	rm -rf *.o kernel.elf neuralkernel.iso iso