CC=gcc
LD=gcc
STRIP=strip -s
WARNINGS=-Wall -Wextra
DEBUG=
COPTIM=-march=native -O2 -pipe
DEFINES=
INCLUDES=
CFLAGS=$(WARNINGS) $(COPTIM) $(DEFINES) $(INCLUDES) $(DEBUG)
LDOPTIM=-Wl,-O1 -Wl,--as-needed
LIBFILES=-levent_core
LDFLAGS=$(WARNINGS) $(LDOPTIM) $(LIBFILES) $(DEBUG)
SRC_DIR=.
BUILD_DIR=build
EXECUTABLE=stream-broadcast

all: $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(EXECUTABLE): $(BUILD_DIR)/stream-broadcast.o $(BUILD_DIR)/stack.o
	$(LD) -o $@ $^ $(LDFLAGS)
	$(STRIP) $@

$(BUILD_DIR)/stream-broadcast.o: $(SRC_DIR)/stream-broadcast.c $(SRC_DIR)/stack.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

$(BUILD_DIR)/stack.o: $(SRC_DIR)/stack.c $(SRC_DIR)/stack.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)



stack_test_: $(BUILD_DIR) stack_test

stack_test: $(BUILD_DIR)/stack_test.o $(BUILD_DIR)/stack.o
	$(LD) -o $@ $^ $(WARNINGS) $(LDOPTIM) $(DEBUG)

$(BUILD_DIR)/stack_test.o: $(SRC_DIR)/stack_test.c $(SRC_DIR)/stack.h Makefile
	$(CC) -o $@ $< -c $(WARNINGS) $(COPTIM) $(DEBUG)



clean:
	rm -rf $(BUILD_DIR)

