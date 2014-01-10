
INC_DIR   := include
INC_FILES := $(shell find $(INC_DIR) -name '*.h')
LIBS      := -lboost_system -lboost_unit_test_framework

.PHONY: all

#all: tests/core tests/sleep tests/all
all: tests/core

run: all
	@./tests/core
	#@./tests/sleep
	#@./tests/all

%: %.cpp $(INC_FILES) Makefile
	@echo $@
	@g++ -I$(INC_DIR) -ggdb -std=c++11 $< $(LIBS) -o $@

clean:
	-rm ./tests/core
	-rm ./tests/sleep
	-rm ./tests/all
