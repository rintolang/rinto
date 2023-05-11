CPP=g++
FRONT-DIR=./src/frontend
DEBUG-DIR=./src/debug-tools
BUILD-DIR=./build
CPP-OPTS=-std=c++14 -Wall
DEPS = -I$(FRONT-DIR) -lmpfr -lgmp

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

build-dir:
	@mkdir -p build
