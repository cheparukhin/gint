#! /usr/bin/make -f
#
#  gint project Makefile
#
#---

#
#  Build configuration
#

# Require configuration file (if you want to clean up and lost the file, you
# can either reconfigure or just delete the build directory)
CONFIG := Makefile.cfg
ifeq "$(wildcard $(CONFIG))" ""
$(error "config file $(CONFIG) does not exist (reconfigure or wipe directory)")
endif
include $(CONFIG)

# Machine flags, defaults are provided for common toolchains

ifeq "$(CONFIG.TOOLCHAIN)" "sh3eb-elf"
machine ?= -m3 -mb
endif
ifeq "$(CONFIG.TOOLCHAIN)" "sh4eb-elf"
machine ?= -m4-nofpu -mb
endif

# Compiler flags, assembler flags, dependency generation, archiving
inc     := -I ../include -I include
cflags  := $(machine) -ffreestanding -nostdlib -Wall -Wextra -std=c11 -Os \
           -fstrict-volatile-bitfields $(inc) $(CONFIG.MACROS) \
           $(CONFIG.CFLAGS)
sflags  := $(inc) $(CONFIG.MACROS)
dflags   = -MMD -MT $@ -MF $(@:.o=.d) -MP
arflags :=

# Git file with current hash
ifeq "$(shell git branch --show-current)" ""
gitfile := ../.git/HEAD
else
gitfile := ../.git/refs/heads/$(shell git branch --show-current)
endif


#
#  File listings
#

# Target file (CONFIG.TARGET is either "fx" or "cg")
target := libgint-$(CONFIG.TARGET).a

# Automatic names for object and dependency files
src2obj = $(1:../src/%=src/%).o
src2dep = $(1:../src/%=src/%).d

# Source files
# TODO: Enable the DMA on fx-9860G
prune-fx := -name render-cg -prune -o -name dma  -prune -o -name r61524 -prune
prune-cg := -name render-fx -prune -o -name gray -prune -o -name t6k11  -prune
src      := $(shell find ../src $(prune-$(CONFIG.TARGET)) \
                    -o -name '*.[csS]' -print)
src_obj  := $(foreach s,$(src),$(call src2obj,$s))

# Files with special handling
spe-fx  := ../src/font5x7.png
spe-cg  := ../src/font8x9.png
spe_obj := $(foreach s,$(spe-$(CONFIG.TARGET)),$(call src2obj,$s))

# All object files
obj := $(src_obj) $(spe_obj)


#
#  Toolchain
#

gcc	= $(CONFIG.TOOLCHAIN)-gcc
as	= $(CONFIG.TOOLCHAIN)-as
ld	= $(CONFIG.TOOLCHAIN)-ld
ar	= $(CONFIG.TOOLCHAIN)-ar
objcopy	= $(CONFIG.TOOLCHAIN)-objcopy


#
#  Version management
#

# Version symbol is obtained by using the last commit hash on 7 nibbles
version_hash = 0x0$(shell git rev-parse --short HEAD)
# Version number: closest tag, with additional commits
version_number = $(shell git describe --tag --always | cut -d- -f1-2)


#
#  Build rules
#

all: $(target)

$(target): include/gint/config.h $(obj)
	@ rm -f $@
	$(call cmd_l,ar,$@) $(ar) rcs $(arflags) $@ $^

# Assembler sources
src/%.s.o: ../src/%.s src/%.s.d
	@ mkdir -p $(dir $@)
	$(call cmd_b,as,$*.s) $(gcc) -c $< -o $@ $(sflags)
src/%.S.o: ../src/%.S src/%.S.d
	@ mkdir -p $(dir $@)
	$(call cmd_b,as,$*.S) $(gcc) -c $< -o $@ $(sflags)

# C sources
src/%.c.o: ../src/%.c src/%.c.d
	@ mkdir -p $(dir $@)
	$(call cmd_b,gcc,$*.c) $(gcc) -c $< -o $@ $(dflags) $(cflags)

# Special files
include/gint/config.h: ../include/gint/config.h.in $(gitfile)
	@ mkdir -p $(dir $@)
	$(call cmd_m,sed,$@) cp $< $@
	@ sed -i'' 's/@GINT_VERSION@/"$(version_number)"/' $@
	@ sed -i'' 's/@GINT_HASH@/$(version_hash)/' $@
$(call src2obj,../src/font5x7.png): ../src/font5x7.png
	@ mkdir -p $(dir $@)
	$(call cmd_m,fxconv,font5x7.png) fxconv -f $< -o $@ \
		--fx --toolchain=$(CONFIG.TOOLCHAIN) name:gint_font5x7 \
		charset:ascii grid.size:5x7 grid.padding:1 grid.border:0
$(call src2obj,../src/font8x9.png): ../src/font8x9.png
	@ mkdir -p $(dir $@)
	$(call cmd_m,fxconv,font8x9.png) fxconv -f $< -o $@ \
		--cg --toolchain=$(CONFIG.TOOLCHAIN) name:gint_font8x9 \
		charset:print grid.size:8x11 grid.padding:1 grid.border:0 \
		proportional:true height:9

#
#  Cleaning
#

clean:
	@ rm -rf src include
distclean: clean
	@ rm -rf Makefile $(CONFIG) $(target)

#
#  Installing
#

m644 := -m 644

# Disable -m on Mac OS
ifeq "$(shell uname)" "Darwin"
m644 :=
endif

install: $(target)
	install -d $(PREFIX)
	install $(target) $(m644) $(PREFIX)
	install ../$(CONFIG.TARGET.LONG).ld $(m644) $(PREFIX)
	install -d $(PREFIX)/include/gint
	cp -r ../include/gint/* $(PREFIX)/include/gint/
	cp -r include/gint/* $(PREFIX)/include/gint/

uninstall:
	rm -f $(PREFIX)/$(target)
	rm -f $(PREFIX)/$(CONFIG.TARGET.LONG).ld
	rm -rf $(PREFIX)/include/gint

#
#  Utilities
#

# Directories: make conveniently leaves a '/' at the end of $(dir ...)
%/:
	@ mkdir -p $@
# Don't try to unlink directories once they're built (that wouldn't work =p)
.PRECIOUS: %/

# Dependency information
-include $(shell [ -d src ] && find src -name *.d)
src/%.d: ;
.PRECIOUS: src/%.d

.PHONY: all clean distclean

# Do not output full commands by default
VERBOSE	?=

# Simple command output method
#   $1  Program name
#   $2  Argument string to display
#   $3  Command color
define cmd
@ echo -e "\e[""$3"";1m>\e[0;1m $1\e[0m $2"
$(if $(VERBOSE),,@)
endef

# Some pre-colored command kinds: misc, build, link, clean, install
cmd_m	= $(call cmd,$1,$2,30)
cmd_b	= $(call cmd,$1,$2,32)
cmd_l	= $(call cmd,$1,$2,36)
cmd_c	= $(call cmd,$1,$2,31)
cmd_i	= $(call cmd,$1,$2,33)
