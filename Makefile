PLUGIN   = super-ws-overlay.so
SRC      = src/main.cpp
CXX     ?= g++
CXXFLAGS = -std=c++2b -shared -fPIC -O2 \
           $(shell pkg-config --cflags hyprland) \
           -DWLR_USE_UNSTABLE

PREFIX  ?= $(HOME)/.config/hypr/plugins

all: $(PLUGIN)

$(PLUGIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

install: $(PLUGIN)
	install -Dm755 $(PLUGIN) $(PREFIX)/$(PLUGIN)

clean:
	rm -f $(PLUGIN)

.PHONY: all install clean
