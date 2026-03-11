CPP=g++
FRONT-DIR=./src/frontend
DEBUG-DIR=./src/debug-tools
BUILD-DIR=./build
CPP-OPTS=-std=c++14 -Wall
DEPS = -I$(FRONT-DIR) -lmpfr -lgmp
PREFIX=/usr/local

FRONTEND_SRC=$(FRONT-DIR)/diagnostic.cc $(FRONT-DIR)/file.cc 	      \
					 $(FRONT-DIR)/operators.cc $(FRONT-DIR)/scanner.cc      \
					 $(FRONT-DIR)/expressions.cc $(FRONT-DIR)/statements.cc \
					 $(FRONT-DIR)/parser.cc

debug-scanner: build-dir
	$(CPP) $(CPP-OPTS) -o $(BUILD-DIR)/debug-scanner.out $(DEBUG-DIR)/scanner.cc \
	$(DEBUG-DIR)/debug-diagnostic.cc -I$(DEBUG-DIR) $(FRONTEND_SRC) $(DEPS)

debug-parser: build-dir
	$(CPP) $(CPP-OPTS) -o $(BUILD-DIR)/debug-parser.out $(DEBUG-DIR)/parser.cc \
	$(DEBUG-DIR)/debug-diagnostic.cc -I$(DEBUG-DIR) $(FRONTEND_SRC) $(DEPS)

test-scanner: build-dir
	$(CPP) $(CPP-OPTS) -o $(BUILD-DIR)/test-scanner.out $(DEBUG-DIR)/test-scanner.cc \
	$(DEBUG-DIR)/debug-diagnostic.cc -I$(DEBUG-DIR) $(FRONTEND_SRC) $(DEPS)

test-parser: build-dir
	$(CPP) $(CPP-OPTS) -o $(BUILD-DIR)/test-parser.out $(DEBUG-DIR)/test-parser.cc \
	$(DEBUG-DIR)/debug-diagnostic.cc -I$(DEBUG-DIR) $(FRONTEND_SRC) $(DEPS)

test: test-scanner test-parser
	$(BUILD-DIR)/test-scanner.out
	$(BUILD-DIR)/test-parser.out

all: debug-scanner debug-parser test-scanner test-parser

install: debug-scanner debug-parser
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(BUILD-DIR)/debug-scanner.out $(DESTDIR)$(PREFIX)/bin/rin-debug-scanner
	install -m 755 $(BUILD-DIR)/debug-parser.out $(DESTDIR)$(PREFIX)/bin/rin-debug-parser

build-dir:
	@mkdir -p build

clean:
	rm -rf $(BUILD-DIR)

.PHONY: all clean build-dir debug-scanner debug-parser test-scanner test-parser test install
