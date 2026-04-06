PLUGIN  = super-ws-overlay.so
SRC     = main.cpp
CXX    ?= g++
CXXFLAGS = -std=c++2b -shared -fPIC -O2 \
           $(shell pkg-config --cflags hyprland) \
           -DWLR_USE_UNSTABLE

PREFIX ?= $(HOME)/.config/hypr/plugins
EWW_DIR ?= $(HOME)/.config/eww

all: $(PLUGIN)

$(PLUGIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

install: $(PLUGIN)
	install -Dm755 $(PLUGIN) $(PREFIX)/$(PLUGIN)
	@echo "Installed $(PLUGIN) to $(PREFIX)/"
	@echo "Copy eww widgets:  cp eww/*.yuck eww/*.scss to your eww config"
	@echo "Then add to hyprland.conf:  plugin = $(PREFIX)/$(PLUGIN)"

clean:
	rm -f $(PLUGIN)

.PHONY: all install clean
