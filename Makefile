
INC_DIR     := include
INC_FILES   := $(shell find $(INC_DIR) -name '*.h')
LIBS        := -lboost_system -lboost_unit_test_framework -lboost_thread -lpthread
TEST_DIR    := tests
TESTS       := core post wait all socket_io
EXAMPLE_DIR := examples
EXAMPLES    := chat_client

.PHONY: all run clean

all: $(addprefix $(TEST_DIR)/, $(TESTS)) \
	   $(addprefix $(EXAMPLE_DIR)/, $(EXAMPLES))

run: all
	@for t in $(TESTS); do $(TEST_DIR)/$$t || exit 1; done
	@./examples/chat_client localhost 9999

%: %.cpp $(INC_FILES) Makefile
	@echo $@
	@g++ -I$(INC_DIR) -Wall -ggdb -std=c++11 $< $(LIBS) -o $@

clean:
	@-for t in $(TESTS); do rm $(TEST_DIR)/$$t; done
