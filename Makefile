CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
SFML_LIBS ?= -lsfml-graphics -lsfml-window -lsfml-system

BIN_DIR ?= build
CONNECT4 := $(BIN_DIR)/connect4
TESTER := $(BIN_DIR)/tester

.PHONY: all connect4 tester clean

all: $(CONNECT4) $(TESTER)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(CONNECT4): main.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(SFML_LIBS)

$(TESTER): tester.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

connect4: $(CONNECT4)

tester: $(TESTER)

clean:
	rm -rf $(BIN_DIR)
