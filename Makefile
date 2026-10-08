# 1. OS Detection Logic
ifeq ($(OS),Windows_NT)
    # Windows settings
    TARGET = quiz_game.exe
    RM = if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR) && if exist $(TARGET) del /f /q $(TARGET)
    MKDIR = if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
    RUN_CMD = .$(subst /,\,/$(TARGET))
else
    # Linux / macOS settings
    TARGET = quiz_game
    RM = rm -rf $(OBJ_DIR) $(TARGET)
    MKDIR = mkdir -p $(OBJ_DIR)
    RUN_CMD = ./$(TARGET)
endif

# 2. Compiler Settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pedantic -O2

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@$(MKDIR)

run: $(TARGET)
	$(RUN_CMD)

clean:
	@$(RM)
