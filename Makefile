CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -D_DEFAULT_SOURCE -Iinclude -pthread
LDFLAGS ?= -pthread

APP = secretguard
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

build_dir:
	mkdir -p build

build/%.o: src/%.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

run: $(APP)
	./$(APP) examples/demo_repo

clean:
	rm -rf build $(APP)

.PHONY: all clean run
