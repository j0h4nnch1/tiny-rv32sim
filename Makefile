WORK_DIR := $(shell pwd)
TARGET := riscv-simulator
SRC := $(WORK_DIR)/src
BUILD_DIR := $(WORK_DIR)/build
INC_DIR := $(WORK_DIR)/include
INCLUDES = $(addprefix -I, $(INC_DIR))
#close warning for now
CXXFLAGS := -std=c++11 -Wall -Wextra -w -g $(INCLUDES)
LDFLAGS := -lreadline

SRCS := $(wildcard $(SRC)/**/*.cpp) $(wildcard $(SRC)/*.cpp)
OBJS := $(patsubst $(SRC)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all clean work

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX)  $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

work:
	@echo $(WORK_DIR)