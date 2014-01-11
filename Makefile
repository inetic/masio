
INC_DIR   := include
INC_FILES := $(shell find $(INC_DIR) -name '*.h')
LIBS      := -lboost_system -lboost_unit_test_framework
TEST_DIR  := tests
TESTS     := core post sleep all

.PHONY: all run clean

all: $(addprefix $(TEST_DIR)/, $(TESTS))

run: all
	@for t in $(TESTS); do $(TEST_DIR)/$$t || exit 1; done

%: %.cpp $(INC_FILES) Makefile
	@echo $@
	@g++ -I$(INC_DIR) -ggdb -std=c++11 $< $(LIBS) -o $@

clean:
	@-for t in $(TESTS); do rm $(TEST_DIR)/$$t; done
