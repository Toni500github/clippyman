CXX       	?= g++
PREFIX	  	?= /usr
MANPREFIX	?= $(PREFIX)/share/man
APPPREFIX 	?= $(PREFIX)/share/applications
VARS  	  	?=

DEBUG 		?= 1

PLATFORM	?= both

# https://stackoverflow.com/a/1079861
# WAY easier way to build debug and release builds
ifeq ($(DEBUG), 1)
        BUILDDIR  = build/debug
        CXXFLAGS := -ggdb3 -Wall -Wextra -Wpedantic -DDEBUG=1 $(DEBUG_CXXFLAGS) $(CXXFLAGS)
else
	# Check if an optimization flag is not already set
	ifneq ($(filter -O%,$(CXXFLAGS)),)
    		$(info Keeping the existing optimization flag in CXXFLAGS)
	else
    		CXXFLAGS := -O3 $(CXXFLAGS)
	endif
        BUILDDIR  = build/release
endif

ifeq ($(filter xorg wayland both,$(PLATFORM)),)
    $(error Invalid platform "$(PLATFORM)". Choose either: xorg, wayland, both)
endif

NAME		= clippyman
TARGET		= clippyman
OLDVERSION	= 0.0.0
VERSION    	= 0.0.1
BRANCH     	= $(shell git rev-parse --abbrev-ref HEAD)
SRC 	   	= $(wildcard src/*.cpp src/clipboard/x11/*.cpp src/clipboard/wayland/*.cpp)
OBJ 	   	= $(SRC:.cpp=.o)
LDFLAGS   	+= -L./$(BUILDDIR)/fmt -lfmt
CXXFLAGS  	?= -mtune=generic -march=native
CXXFLAGS        += -fvisibility=hidden -Iinclude -std=c++20 $(VARS) -DVERSION=\"$(VERSION)\" -DBRANCH=\"$(BRANCH)\"

ifeq ($(PLATFORM),xorg)
	LDFLAGS  += -lxcb -lxcb-xfixes
	CXXFLAGS += -DPLATFORM_WAYLAND=0 -DPLATFORM_XORG=1
	TARGET   := $(TARGET)-xorg
endif
ifeq ($(PLATFORM),wayland)
	LDFLAGS  += -lwayland-client
	CXXFLAGS += -DPLATFORM_WAYLAND=1 -DPLATFORM_XORG=0
	TARGET   := $(TARGET)-wayland
endif

all: fmt toml both protocol $(TARGET)

fmt:
ifeq ($(wildcard $(BUILDDIR)/fmt/libfmt.a),)
	mkdir -p $(BUILDDIR)/fmt
	make -C src/fmt BUILDDIR=$(BUILDDIR)/fmt
endif

toml:
ifeq ($(wildcard $(BUILDDIR)/toml++/toml.o),)
	mkdir -p $(BUILDDIR)/toml++
	make -C src/toml++ BUILDDIR=$(BUILDDIR)/toml++
endif

protocol:
ifeq ($(PLATFORM),wayland)
	wayland-scanner private-code src/clipboard/wayland/protocol/wlr-data-control-unstable-v1.xml include/clipboard/wayland/wlr-data-control-unstable-v1.h
	wayland-scanner client-header src/clipboard/wayland/protocol/wlr-data-control-unstable-v1.xml src/clipboard/wayland/wlr-data-control-unstable-v1.c
endif

$(TARGET): fmt toml protocol $(OBJ)
ifneq ($(PLATFORM),both)
	mkdir -p $(BUILDDIR)
	$(CXX) $(OBJ) $(BUILDDIR)/toml++/toml.o -o $(BUILDDIR)/$(TARGET) $(LDFLAGS)
endif

both:
ifeq ($(PLATFORM),both)
	make clean && rm -f $(BUILDDIR)/$(TARGET)-wayland && make PLATFORM=wayland -j4
	make clean && rm -f $(BUILDDIR)/$(TARGET)-xorg && make PLATFORM=xorg -j4
endif

dist:
	bsdtar -zcf $(NAME)-v$(VERSION).tar.gz LICENSE $(TARGET).1 -C $(BUILDDIR) $(TARGET)

clean:
	rm -rf $(BUILDDIR)/$(TARGET) $(OBJ)

distclean:
	rm -rf $(BUILDDIR) $(OBJ)
	find . -type f -name "*.tar.gz" -exec rm -rf "{}" \;
	find . -type f -name "*.o" -exec rm -rf "{}" \;
	find . -type f -name "*.a" -exec rm -rf "{}" \;

updatever:
	sed -i "s#$(OLDVERSION)#$(VERSION)#g" $(wildcard .github/workflows/*.yml) compile_flags.txt

.PHONY: $(TARGET) updatever dist fmt toml distclean install all
