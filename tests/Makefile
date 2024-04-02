CROSS_COMPILE := riscv64-linux-gnu-
# CROSS_COMPILE = $(BUILD_ROOT)riscv32-buildroot-linux-uclibc-

CXX := $(CROSS_COMPILE)g++
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
DP := $(CROSS_COMPILE)objdump
AS := $(CROSS_COMPILE)as
OBJCOPY := $(CROSS_COMPILE)objcopy

CXXFLAGS := -march=rv32ima -mabi=ilp32 -static -std=c++11 -O0
CCFLAGS := -march=rv32ima -mabi=ilp32 -static -std=c11 -O2
LDFLAGS := -melf32lriscv -T linker.ld
ASFLAGS := -fno-pic -march=rv32im -mabi=ilp32

SRC_DIR := $(shell pwd)

BUILD_DIR := $(SRC_DIR)/build

SRCS := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.S) $(wildcard $(SRC_DIR)/*.c)

OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(filter %.cpp,$(SRCS))) \
		$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter %.c,$(SRCS))) \
        $(patsubst $(SRC_DIR)/%.S, $(BUILD_DIR)/%.o, $(filter %.S,$(SRCS)))

TARGET := test

all: $(TARGET)

show:
	@echo $(OBJS)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$@ $^
	$(OBJCOPY) -O elf32-littleriscv $(BUILD_DIR)/$@ $(BUILD_DIR)/$@.elf
	$(DP) -S $(BUILD_DIR)/$@.elf > $(BUILD_DIR)/test.S

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)




