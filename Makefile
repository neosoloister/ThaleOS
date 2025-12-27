NASM = nasm
QEMU = qemu-system-i386

SRC_DIR = src
BUILD_DIR = build

BOOT_ASM = $(SRC_DIR)/boot.asm
BOOT_BIN = $(BUILD_DIR)/boot.bin
IMG      = $(BUILD_DIR)/floppy.img

all: $(BOOT_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BOOT_BIN): $(BOOT_ASM) | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@
	@size=$$(stat -c%s $@); \
	if [ $$size -ne 512 ]; then \
		echo "Error: boot.bin is $$size bytes (must be 512)"; \
		exit 1; \
	fi

$(IMG): $(BOOT_BIN)
	dd if=/dev/zero of=$(IMG) bs=512 count=2880 status=none
	dd if=$(BOOT_BIN) of=$(IMG) conv=notrunc status=none

run: $(IMG)
	$(QEMU) -fda $(IMG)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean