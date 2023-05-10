CPP=g++
FRONT-DIR=./src/frontend
DEBUG-DIR=./src/debug-tools
BUILD-DIR=./build
CPP-OPTS=-std=c++14 -Wall -lmpfr -I$(FRONT-DIR)

FRONTEND_SRC=$(FRONT-DIR)/diagnostic.cc $(FRONT-DIR)/file.cc 	      \
						 $(FRONT-DIR)/operators.cc $(FRONT-DIR)/scanner.cc      \
						 $(FRONT-DIR)/expressions.cc $(FRONT-DIR)/statements.cc \
						 $(FRONT-DIR)/parser.cc

debug-scanner: build-dir
	$(CPP) $(CPP-OPTS) -I$(DEBUG-DIR) -o $(BUILD-DIR)/a.out $(DEBUG-DIR)/scanner.cc $(DEBUG-DIR)/debug-diagnostic.cc $(FRONTEND_SRC)

debug-parser: build-dir
	$(CPP) $(CPP-OPTS) -I$(DEBUG-DIR) -o $(BUILD-DIR)/a.out $(DEBUG-DIR)/parser.cc $(DEBUG-DIR)/debug-diagnostic.cc $(FRONTEND_SRC)

build-dir:
	@mkdir -p build
