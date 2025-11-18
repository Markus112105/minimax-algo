CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

# Prefer pkg-config discovery when SFML is installed outside default include/lib paths
SFML_PKGCONFIG_CFLAGS := $(shell pkg-config --cflags sfml-graphics 2>/dev/null)
SFML_PKGCONFIG_LIBS := $(shell pkg-config --libs sfml-graphics 2>/dev/null)

ifneq ($(strip $(SFML_PKGCONFIG_CFLAGS)),)
CXXFLAGS += $(SFML_PKGCONFIG_CFLAGS)
endif

ifeq ($(strip $(SFML_PKGCONFIG_LIBS)),)
SFML_LIBS ?= -lsfml-graphics -lsfml-window -lsfml-system
else
SFML_LIBS ?= $(SFML_PKGCONFIG_LIBS)
endif

BIN_DIR ?= build
CONNECT4 := $(BIN_DIR)/connect4

.PHONY: all connect4 clean

all: $(CONNECT4)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(CONNECT4): main.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(SFML_LIBS)

connect4: $(CONNECT4)

clean:
	rm -rf $(BIN_DIR)
