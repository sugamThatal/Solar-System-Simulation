# ─────────────────────────────────────────────
#  Solar System — Makefile (macOS)
# ─────────────────────────────────────────────

CXX    = g++
TARGET = SolarSystem

# Apple Silicon vs Intel
UNAME := $(shell uname -m)
ifeq ($(UNAME), arm64)
    BREW_PREFIX = /opt/homebrew
else
    BREW_PREFIX = /usr/local
endif

INCLUDES = -I$(BREW_PREFIX)/include
LIBDIRS  = -L$(BREW_PREFIX)/lib

CXXFLAGS = -std=c++17 -O2 $(INCLUDES) -Wno-deprecated-declarations
LDFLAGS  = $(LIBDIRS) -lglfw \
           -framework OpenGL -framework Cocoa \
           -framework IOKit  -framework CoreVideo

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $@ $(LDFLAGS)
	@echo ""
	@echo "Build successful! Run with:  ./$(TARGET)"
	@echo ""

clean:
	rm -f $(TARGET)

.PHONY: all clean
