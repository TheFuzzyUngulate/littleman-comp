CXX = g++
CXXFLAGS = -g
DELETE :=
	ifeq ($(OS),Windows_NT)
		DELETE += del lmc.exe
	else
		DELETE += rm -f lmc
	endif

all: lmc

lmc: src/main.cpp src/processor.hpp src/scanner.hpp
	$(CXX) $(CXXFLAGS) $< -o lmc

clean:
	$(DELETE)