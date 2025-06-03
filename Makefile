# User vars
GDB = 0
DISAS_IN_FILE = 0
# User targets:
.PHONY: run 
.PHONY: compile
.PHONY: clean 
.PHONY: disas 
#
default: run

BUILD_DIR = build
SRC_DIR = src

QEMU = qemu-system-riscv32

CC = clang 
CFLAGS += -std=c11 -O2 -g3 -Wall -Wextra  -fno-stack-protector -ffreestanding -nostdlib --target=riscv32-unknown-elf
QEMUFLAGS += -machine virt
QEMUFLAGS += -bios default
QEMUFLAGS += -nographic
QEMUFLAGS += -serial mon:stdio
QEMUFLAGS += --no-reboot

ifeq ($(GDB),1)
	QEMUFLAGS += -S 
	QEMUFLAGS += -gdb 
	QEMUFLAGS += tcp::1234
endif


LINKER_SCRIPT=$(SRC_DIR)/kernel.ld
C_FILES+= $(wildcard $(SRC_DIR)/*.c)
H_FILES+= $(wildcard $(SRC_DIR)/*.h)

KERNEL=$(BUILD_DIR)/kernel.elf
DISAS=$(BUILD_DIR)/kernel.S


$(BUILD_DIR): 
	mkdir -pv $@

compile: $(KERNEL)


$(KERNEL): $(BUILD_DIR) $(C_FILES) $(LINKER_SCRIPT) $(H_FILES)
	$(CC) \
	$(CFLAGS) \
	-Wl,-Map=$(BUILD_DIR)/kernel.map \
	-Wl,-T$(LINKER_SCRIPT) \
	-o $(KERNEL) \
	$(C_FILES)

run: $(KERNEL)
	$(QEMU) $(QEMUFLAGS) -kernel $(KERNEL)  

disas: $(DISAS)

ifeq ($(DISAS_IN_FILE),1)
	DISAS_FLAG += >> $@
endif

$(DISAS): $(KERNEL)
	llvm-objdump -d  $(KERNEL)  $(DISAS_FLAG)

clean:
	rm -rf build