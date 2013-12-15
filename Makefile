
INC_DIR   := include
INC_FILES := $(shell find $(INC_DIR) -name '*.h')
LIBS      := -lboost_system -lboost_unit_test_framework

.PHONY: all

all: tests/simple_run

run: all
	@./tests/simple_run

%: %.cpp $(INC_FILES) Makefile
	@echo $@
	@g++ -I$(INC_DIR) -ggdb -std=c++11 $< $(LIBS) -o $@


