#
# Copyright (c) Aaron Delasy
#
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential
#

PROJECT_NAME=the-core
BUILD_DIR=build
SOURCE_DIR=src
SOURCES=src/error.c src/lexer.c src/main.c src/reader.c src/token.c

CC=gcc
CFLAGS=-Wall -Wconversion -Werror -Wextra -Wparentheses -Wshadow -Wundef -Wunreachable-code -pedantic-errors -std=c11 -g
MKDIR=mkdir -p
OBJECTS=$(SOURCES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)
RM=rm -f

all: dir $(BUILD_DIR)/$(PROJECT_NAME)
.PHONY: all

clean:
	$(RM) $(BUILD_DIR)/*.o $(BUILD_DIR)/$(PROJECT_NAME)
.PHONY: clean

dir:
	$(MKDIR) $(BUILD_DIR)
.PHONY: dir

$(OBJECTS): $(BUILD_DIR)/%.o : $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(PROJECT_NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: all clean dir
