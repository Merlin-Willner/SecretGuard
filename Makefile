CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -D_DEFAULT_SOURCE -Iinclude -pthread
LDFLAGS ?= -pthread

APP = secretguard
SRC = $(wildcard src/*.c)
SRC_NO_MAIN = $(filter-out src/main.c,$(SRC))
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))
TEST_DIR = tests
UNITY_DIR ?= tests/third_party/unity
UNITY_SRC ?= $(UNITY_DIR)/unity.c
UNITY_HEADER ?= $(UNITY_DIR)/unity.h
UNITY_INTERNALS ?= $(UNITY_DIR)/unity_internals.h
TEST_BIN = build/test_all
TEST_SRC = $(wildcard $(TEST_DIR)/*.c) $(SRC_NO_MAIN)
TEST_CFLAGS = $(filter-out -Werror,$(CFLAGS)) -I$(UNITY_DIR)
TEST_UNITY_DEPS = $(UNITY_SRC) $(UNITY_HEADER) $(UNITY_INTERNALS)

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

build_dir:
	mkdir -p build

build/%.o: src/%.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

run: $(APP)
	./$(APP) 

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(TEST_UNITY_DEPS) | build_dir
	$(CC) $(TEST_CFLAGS) $(TEST_SRC) $(UNITY_SRC) $(LDFLAGS) -o $@

clean:
	rm -rf build $(APP)

.PHONY: all clean run test
