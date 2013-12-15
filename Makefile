
INC_DIR := include

tests/simple_run: tests/simple_run.cpp
	g++ -I$(INC_DIR) -ggdb -std=c++11 tests/simple_run.cpp \
		-lboost_system \
	  -lboost_unit_test_framework \
		-o tests/simple_run


