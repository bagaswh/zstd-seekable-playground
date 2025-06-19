# Makefile for CLI project with zstd submodule

# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -pedantic -Werror -O2 -std=c17 -march=native
DEBUG_CFLAGS = -Wall -Wextra -g -DDEBUG -std=c99
ASAN_CFLAGS = -Wall -Wextra -g -DDEBUG -std=c99 -fsanitize=address -fno-omit-frame-pointer

# Linker flags
LDFLAGS = 
ASAN_LDFLAGS = -fsanitize=address

# Project structure
BUILD_DIR = build
TARGET = $(BUILD_DIR)/cli
SOURCES = cli.c
SOURCE_FILES = common.c

# Zstd paths
ZSTD_DIR = external/zstd
ZSTD_LIB_DIR = $(ZSTD_DIR)/lib
ZSTD_LIB = $(ZSTD_LIB_DIR)/libzstd.a

# Seekable zstd sources
ZSTD_SEEKABLE_DIR = $(ZSTD_DIR)/contrib/seekable_format
ZSTD_SEEKABLE_SRCS = \
  $(ZSTD_SEEKABLE_DIR)/zstdseek_compress.c \
  $(ZSTD_SEEKABLE_DIR)/zstdseek_decompress.c
ZSTD_SEEKABLE_OBJS = $(ZSTD_SEEKABLE_SRCS:.c=.o)

# Include paths
INCLUDES = -I$(ZSTD_LIB_DIR) -I$(ZSTD_LIB_DIR)/common -I$(ZSTD_SEEKABLE_DIR)

# Default target
all: $(TARGET)

# Build output directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build zstd static library
$(ZSTD_LIB):
	@echo "Building libzstd.a..."
	$(MAKE) -C $(ZSTD_LIB_DIR) libzstd.a

# Compile seekable source files into .o
$(ZSTD_SEEKABLE_DIR)/%.o: $(ZSTD_SEEKABLE_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Build main target
$(TARGET): $(BUILD_DIR) $(ZSTD_LIB) $(ZSTD_SEEKABLE_OBJS) $(SOURCES)
	@echo "Building CLI binary..."
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) $(ZSTD_SEEKABLE_OBJS) $(ZSTD_LIB) $(SOURCE_FILES) $(LDFLAGS) -o $(TARGET)

# Debug build
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: all

# ASan build
asan: CFLAGS = $(ASAN_CFLAGS)
asan: LDFLAGS = $(ASAN_LDFLAGS)
asan: TARGET = $(BUILD_DIR)/cli-asan
asan: clean-objs $(TARGET)
	@echo "Built with AddressSanitizer: $(TARGET)"

# Clean only object files (preserves zstd lib)
clean-objs:
	@echo "Cleaning object files..."
	find $(ZSTD_SEEKABLE_DIR) -name '*.o' -delete

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	find $(ZSTD_SEEKABLE_DIR) -name '*.o' -delete

# Clean everything including zstd
clean-all: clean
	@echo "Cleaning zstd static library..."
	$(MAKE) -C $(ZSTD_LIB_DIR) clean

# Initialize submodules (for new clones)
init-submodules:
	git submodule update --init --recursive

# Run the binary
run: all
	$(TARGET)

# Run with ASan
run-asan: asan
	$(BUILD_DIR)/cli-asan

# Phony targets
.PHONY: all debug asan clean clean-objs clean-all init-submodules run run-asan