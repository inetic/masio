
INC_DIR     := ../include
INC_FILES   := $(shell find $(INC_DIR) -name '*.h')
LIBS        := -lboost_system -lboost_thread -lpthread
EXAMPLES    := chat_client

.PHONY: all run clean

all: $(EXAMPLES)

run: all
	@gdb -ex=r --args ./chat_client localhost 9999

%: %.cpp $(INC_FILES) Makefile
	@echo $@
	@g++ -I$(INC_DIR) -Wall -ggdb -std=c++11 $< $(LIBS) -o $@

clean:
	@-for t in $(EXAMPLES); do rm ./$$t; done
