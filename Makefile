# ===== ThaleOS / hobby x86 bootloader + kernel build =====
# Layout (LBA, 512 bytes/sector):
#   0  : mbr.bin (512 bytes)
#   1..(1+STAGE2_PAD_SECTORS-1) : stage2.bin (padded to fixed size)
#   KERNEL_LBA (= 1 + STAGE2_PAD_SECTORS) ... : kernel.bin (auto sized)

# ---- tools ----
NASM       := nasm
CC         := i686-elf-gcc
LD         := i686-elf-ld
OBJCOPY    := i686-elf-objcopy
QEMU       := qemu-system-i386

# ---- paths ----
BUILD      := build
IMG        := $(BUILD)/os.img

MBR_ASM    := src/boot/mbr.asm
STAGE2_ASM := src/boot/stage2.asm
KENTRY_ASM := src/boot/kernel_entry.asm

LINKER_LD  := linker.ld

MBR_BIN    := $(BUILD)/mbr.bin
STAGE2_BIN := $(BUILD)/stage2.bin

KENTRY_O   := $(BUILD)/kernel_entry.o

KERNEL_ELF := $(BUILD)/kernel.elf
KERNEL_BIN := $(BUILD)/kernel.bin

# ---- constants ----
SECTOR_SIZE        := 512
STAGE2_PAD_SECTORS := 16                 # stage2 is padded to fixed size (8 KiB)
KERNEL_LBA         := $(shell echo $$((1 + $(STAGE2_PAD_SECTORS))))

# ---- flags ----
CFLAGS := -std=gnu11 -O2 -g -ffreestanding -fno-pie -fno-pic -fno-stack-protector \
          -Wall -Wextra -Werror -m32 -I./src -I./src/lib -I./src/cpu

LDFLAGS := -m elf_i386 -T $(LINKER_LD)

# ---- auto collect kernel C sources (everything except src/boot) ----
C_SRCS := $(shell find src -type f -name '*.c' ! -path 'src/boot/*')
C_OBJS := $(patsubst src/%.c,$(BUILD)/%.o,$(C_SRCS))

.PHONY: all run clean print-vars

all: $(IMG)

# =========================
# Linker script (1 MiB load)
# =========================
# If you already have your own linker.ld, delete this rule or replace content.
$(LINKER_LD):
	@printf '%s\n' \
'ENTRY(_start)' \
'' \
'SECTIONS {' \
'  . = 0x00100000;' \
'  .text   : { *(.text*) }' \
'  .rodata : { *(.rodata*) }' \
'  .data   : { *(.data*) }' \
'  .bss    : { *(COMMON) *(.bss*) }' \
'}' > $(LINKER_LD)

# =========
# Build dir
# =========
$(BUILD):
	mkdir -p $(BUILD)

# =========
# Boot parts
# =========
$(MBR_BIN): $(MBR_ASM) | $(BUILD)
	$(NASM) -f bin \
	  -D__STAGE2_SECTORS__=$(STAGE2_PAD_SECTORS) \
	  $(MBR_ASM) -o $(MBR_BIN)
	@test "$$(stat -c%s $(MBR_BIN))" -eq 512

# stage2 uses auto kernel sector count computed from kernel.bin size.
# stage2 is padded to STAGE2_PAD_SECTORS so KERNEL_LBA is constant.
$(STAGE2_BIN): $(STAGE2_ASM) $(KERNEL_BIN) | $(BUILD)
	@KERNEL_SIZE=$$(stat -c%s $(KERNEL_BIN)); \
	KERNEL_SECTORS=$$(( (KERNEL_SIZE + $(SECTOR_SIZE) - 1) / $(SECTOR_SIZE) )); \
	echo "[stage2] kernel.bin: $$KERNEL_SIZE bytes => $$KERNEL_SECTORS sectors, KERNEL_LBA=$(KERNEL_LBA)"; \
	$(NASM) -f bin \
	  -D__KERNEL_LBA__=$(KERNEL_LBA) \
	  -D__KERNEL_SECTORS__=$$KERNEL_SECTORS \
	  $(STAGE2_ASM) -o $(STAGE2_BIN); \
	truncate -s $$(( $(STAGE2_PAD_SECTORS) * $(SECTOR_SIZE) )) $(STAGE2_BIN)

# ======
# Kernel
# ======
$(KENTRY_O): $(KENTRY_ASM) | $(BUILD)
	$(NASM) -f elf32 $(KENTRY_ASM) -o $(KENTRY_O)

# Pattern rule: compile any src/.../*.c to build/.../*.o (and create folders)
$(BUILD)/%.o: src/%.c | $(BUILD)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel.elf from entry + all compiled C objects
$(KERNEL_ELF): $(KENTRY_O) $(C_OBJS) $(LINKER_LD) | $(BUILD)
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(KENTRY_O) $(C_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF) | $(BUILD)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)

# ==========
# Disk image
# ==========
$(IMG): $(MBR_BIN) $(STAGE2_BIN) $(KERNEL_BIN) | $(BUILD)
	@KERNEL_SIZE=$$(stat -c%s $(KERNEL_BIN)); \
	KERNEL_SECTORS=$$(( (KERNEL_SIZE + $(SECTOR_SIZE) - 1) / $(SECTOR_SIZE) )); \
	echo "[img] Writing $(IMG)"; \
	echo "[img] stage2 pad: $(STAGE2_PAD_SECTORS) sectors => kernel starts at LBA $(KERNEL_LBA)"; \
	echo "[img] kernel sectors: $$KERNEL_SECTORS"; \
	rm -f $(IMG); \
	dd if=/dev/zero of=$(IMG) bs=1M count=16 status=none; \
	dd if=$(MBR_BIN)    of=$(IMG) conv=notrunc bs=$(SECTOR_SIZE) seek=0 status=none; \
	dd if=$(STAGE2_BIN) of=$(IMG) conv=notrunc bs=$(SECTOR_SIZE) seek=1 status=none; \
	dd if=$(KERNEL_BIN) of=$(IMG) conv=notrunc bs=$(SECTOR_SIZE) seek=$(KERNEL_LBA) status=none

run: $(IMG)
	$(QEMU) -drive format=raw,file=$(IMG)

print-vars:
	@echo "STAGE2_PAD_SECTORS=$(STAGE2_PAD_SECTORS)"
	@echo "KERNEL_LBA=$(KERNEL_LBA)"
	@echo "C_SRCS=$(C_SRCS)"
	@echo "C_OBJS=$(C_OBJS)"

clean:
	rm -rf $(BUILD) $(LINKER_LD)