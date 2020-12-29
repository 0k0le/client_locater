# General compilation flags
UNI_FLAGS = -Wextra -Wall -v
VERSION = release


# Src file locations
SRC_DIR = src
OBJ_DIR = bin-int/$(VERSION)
#OBJ_FILES = $(OBJ_DIR)/main.o $(OBJ_DIR)/func.o

# App info
APP_NAME = client_locater
APP_DIR = bin/$(VERSION)

# Compiler
CC = gcc

debug: VERSION = debug
debug: UNI_FLAGS += -D_DEBUG
debug: UNI_FLAGS += -g
static: UNI_FLAGS += -static
all: $(SRC_FILES)
	$(CC) -c -DUSE_OPENSSL $(SRC_DIR)/main.c $(UNI_FLAGS) -O2 -o bin/int/main.o
	$(CC) -c -DUSE_OPENSSL $(SRC_DIR)/crypt.c $(UNI_FLAGS) -O2 -o bin/int/crypt.o
	$(CC) -c -DUSE_OPENSSL $(SRC_DIR)/func.c $(UNI_FLAGS) -O2 -o bin/int/func.o
	$(CC) $(UNI_FLAGS) -o $(APP_DIR)/$(APP_NAME) -O2 bin/int/main.o bin/int/func.o bin/int/crypt.o -lcurl -lssl -lcrypto -lpthread #./lib/curl/build/lib/.libs/libcurl.a ./lib/zlib/build/libz.a ./lib/openssl/build/libssl.a ./lib/openssl/build/libcrypto.a ./lib/glibc/build/dlfcn/libdl.a ./lib/glibc/build/nptl/libpthread.a ./lib/glibc/build/libc.a

debug: all
static: all

clean:
	rm -f bin/int/*.o
	rm -f bin/debug/$(APP_NAME)
	rm -f bin/release/$(APP_NAME)
