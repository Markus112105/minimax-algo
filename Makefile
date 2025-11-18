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

TARGET ?= connect4

.PHONY: all $(TARGET) clean

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(SFML_LIBS)

clean:
	rm -f $(TARGET)
