# Requires clang + lld-link (brew install llvm on macOS). This VM has neither, so a full EFI link cannot be verified here.
CC = clang
CFLAGS = -target x86_64-unknown-windows -ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone -Wall -Wextra -Iinclude
SRCS = boot/uefi_boot.c kernel/kmain.c kernel/serial.c
OBJS = $(SRCS:.c=.o)
all: BOOTX64.EFI
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
BOOTX64.EFI: $(OBJS)
	$(CC) -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link $(OBJS) -o $@
clean:
	rm -f $(OBJS) BOOTX64.EFI
