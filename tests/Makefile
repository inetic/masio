
INC_DIR     := ../include
INC_FILES   := $(shell find $(INC_DIR) -name '*.h')
LIBS        := -lboost_system -lboost_unit_test_framework
TESTS       := core post wait all socket_io

.PHONY: tests run clean

tests: $(TESTS)

run: all
	@for t in $(TESTS); do ./$$t || exit 1; done

%: %.cpp $(INC_FILES) Makefile
	@echo $@
	@g++ -I$(INC_DIR) -Wall -ggdb -std=c++11 $< $(LIBS) -o $@

clean:
	@-for t in $(TESTS); do rm ./$$t; done
