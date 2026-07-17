CXX := x86_64-elf-g++
CXXFLAGS := -m32 -ffreestanding -fno-exceptions -fno-rtti -fno-pie -fno-pic -fno-stack-protector -fno-threadsafe-statics -fno-use-cxa-atexit -mno-sse -mno-sse2 -mno-mmx
NASM := nasm
LD := ld
OBJCOPY := objcopy

CPP_SOURCES := \
	kernel/kernel.cpp \
	kernel/terminal.cpp \
	kernel/printk.cpp \
	kernel/gdt.cpp \
	kernel/idt.cpp \
	kernel/isr.cpp \
	kernel/pic.cpp \
	kernel/pit.cpp \
	kernel/irq.cpp \
	kernel/keyboard.cpp \
	kernel/shell.cpp \
	kernel/telemetry.cpp \
	kernel/pmm.cpp \
	kernel/paging.cpp \
	kernel/heap.cpp \
	kernel/task.cpp \
	kernel/scheduler.cpp \
	kernel/syscall.cpp \
	kernel/elf.cpp \
	kernel/vfs.cpp \
	kernel/splash.cpp \
	kernel/auth.cpp \
	kernel/editor.cpp \
	kernel/nn.cpp \
	kernel/nn_pmm.cpp \
	kernel/nn_scheduler.cpp \
	kernel/watchdog.cpp

CPP_OBJECTS := $(patsubst kernel/%.cpp,%.o,$(CPP_SOURCES))

ASM_SOURCES := \
	boot/multiboot2.asm \
	boot/boot.asm \
	kernel/context.asm \
	kernel/gdt.asm \
	kernel/idt.asm \
	kernel/irq.asm \
	kernel/keyboard.asm \
	kernel/syscall.asm \
	kernel/user_mode.asm

ASM_OBJECTS := \
	multiboot.o \
	boot.o \
	context.o \
	gdtasm.o \
	idtasm.o \
	irqasm.o \
	keyboardasm.o \
	syscallasm.o \
	user_mode.o

all: kernel.elf

%.o: kernel/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

multiboot.o: boot/multiboot2.asm
	$(NASM) -f elf32 $< -o $@

boot.o: boot/boot.asm
	$(NASM) -f elf32 $< -o $@

context.o: kernel/context.asm
	$(NASM) -f elf32 $< -o $@

gdtasm.o: kernel/gdt.asm
	$(NASM) -f elf32 $< -o $@

idtasm.o: kernel/idt.asm
	$(NASM) -f elf32 $< -o $@

irqasm.o: kernel/irq.asm
	$(NASM) -f elf32 $< -o $@

keyboardasm.o: kernel/keyboard.asm
	$(NASM) -f elf32 $< -o $@

syscallasm.o: kernel/syscall.asm
	$(NASM) -f elf32 $< -o $@

user_mode.o: kernel/user_mode.asm
	$(NASM) -f elf32 $< -o $@

user/hello.o: user/hello.asm
	$(NASM) -f elf32 $< -o $@

user/hello.elf: user/hello.o user/linker.ld
	$(LD) -m elf_i386 -T user/linker.ld -o $@ user/hello.o

tools/mkinitrd: tools/mkinitrd.c
	gcc -O2 -Wall -Wextra $< -o $@

initrd/initrd.nkfs: tools/mkinitrd user/hello.elf $(wildcard nkfs_files/*)
	mkdir -p initrd nkfs_files
	cp user/hello.elf nkfs_files/hello.elf
	tools/mkinitrd $@ nkfs_files

initrd/initrd_bin.o: initrd/initrd.nkfs
	cd initrd && $(OBJCOPY) -I binary -O elf32-i386 -B i386 initrd.nkfs initrd_bin.o

kernel.elf: $(CPP_OBJECTS) $(ASM_OBJECTS) initrd/initrd_bin.o linker.ld
	$(CXX) \
		-m32 \
		-nostdlib \
		-no-pie \
		-Wl,-m,elf_i386 \
		-T linker.ld \
		-o $@ \
		$(ASM_OBJECTS) \
		$(CPP_OBJECTS) \
		initrd/initrd_bin.o
	@if command -v grub-file >/dev/null 2>&1; then grub-file --is-x86-multiboot2 $@ || { echo "ERROR: invalid Multiboot2 kernel"; rm -f $@; exit 1; }; fi

iso: kernel.elf
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/kernel.elf
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o neuralkernel.iso iso

run: iso
	qemu-system-x86_64 -cdrom neuralkernel.iso

debug: iso
	qemu-system-x86_64 -cdrom neuralkernel.iso -no-reboot -no-shutdown -d int,cpu_reset,guest_errors -D qemu.log

clean:
	rm -rf *.o kernel.elf neuralkernel.iso iso
	rm -f user/hello.o user/hello.elf nkfs_files/hello.elf
	rm -f initrd/initrd.nkfs initrd/initrd_bin.o
	rm -f tools/mkinitrd
