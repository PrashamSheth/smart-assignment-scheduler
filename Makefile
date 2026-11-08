CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude

all: smart_scheduler test_scheduler

smart_scheduler: src/main.cpp src/scheduler.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test_scheduler: tests/test_scheduler.cpp src/scheduler.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test: test_scheduler
	./test_scheduler

clean:
	rm -f smart_scheduler test_scheduler *.exe

.PHONY: all test clean
