CXX = g++
CXXFLAGS = -g
DELETE :=
	ifeq ($(OS),Windows_NT)
		DELETE += del bin\lmc.exe
	else
		DELETE += rm -f bin/lmc
	endif

all: lmc

lmc: src/main.cpp src/processor.hpp src/scanner.hpp
	$(CXX) $(CXXFLAGS) $< -o bin/lmc

clean:
	$(DELETE)