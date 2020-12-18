# General compilation flags
UNI_FLAGS = -Wextra -Wall
VERSION = release

# Src file locations
SRC_DIR = src
OBJ_DIR = bin-int/$(VERSION)
#OBJ_FILES = $(OBJ_DIR)/main.o $(OBJ_DIR)/func.o
SRC_FILES = $(SRC_DIR)/func.c $(SRC_DIR)/crypt.c $(SRC_DIR)/main.c

# App info
APP_NAME = client_locater
APP_DIR = bin/$(VERSION)

# Compiler
CC = gcc

debug: VERSION = debug
debug: UNI_FLAGS += -D_DEBUG
all: $(SRC_FILES)
	$(CC) -DUSE_OPENSSL $(SRC_FILES) -lcurl -lssl -lcrypto -lpthread $(UNI_FLAGS) -O2 -o $(APP_DIR)/$(APP_NAME) 

debug: all
