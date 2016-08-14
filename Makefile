#! /usr/bin/make -f
#---
#
#	gint project Makefile.
#
#---



#---
#	Project variables.
#---

# Modules
modules-gint	= core keyboard mmu mpu rtc screen timer \
		  bopti display gray tales
modules-libc	= setjmp string stdio stdlib time

# Targets
target-g1a	= gintdemo.g1a
target-lib	= libgint.a
target-std	= libc.a

# Tools
cc	= sh3eb-elf-gcc
as	= sh3eb-elf-as
ar	= sh3eb-elf-ar
ob	= sh3eb-elf-objcopy
wr	= g1a-wrapper

# Flags
cflags		= -m3 -mb -nostdlib -I include -ffreestanding -std=c11 -Os \
-W -Wall -Wextra -pedantic

# Demo application (could be done better)
demo-src	= $(notdir $(wildcard demo/*.[cs]))
demo-dep	= $(wildcard demo/*.h)
demo-ld		= demo/gintdemo.ld
demo-icon	= demo/icon.bmp
demo-res	= $(notdir $(wildcard demo/resources/*))
demo-obj	= $(patsubst %,build/demo_%.o,$(demo-src) $(demo-res))
demo-elf	= build/gintdemo.elf
demo-bin	= build/gintdemo.bin
demo-libs	= -L. -lgint -lc -lgcc

# Specific objects
obj-lib-spec	= build/display_font_system.bmp.o
obj-std-spec	=



#---
#	Automatic variables.
#---

# Modules are subfolders of src/.
modules = $(modules-gint) $(modules-libc)

define n
# This is a newline character.

endef

# Module-scope variables.
$(foreach mod, $(modules), $(eval \
	mod-$(mod)-c	= $(notdir $(wildcard src/$(mod)/*.c))		$n\
	mod-$(mod)-asm	= $(notdir $(wildcard src/$(mod)/*.s))		$n\
	mod-$(mod)-src	= $$(mod-$(mod)-c)$$(mod-$(mod)-asm)		$n\
	mod-$(mod)-obj	= $$(patsubst %,build/$(mod)_%.o,$$(mod-$(mod)-src)) \
))

# Target-scope variables.
obj-std = $(foreach mod,$(modules-libc),$(mod-$(mod)-obj)) $(obj-std-spec)
obj-lib	= $(foreach mod,$(modules-gint),$(mod-$(mod)-obj)) $(obj-lib-spec)

# Dependencies
hdr-dep = $(wildcard include/*.h include/internals/*.h)



#---
#	Rule templates.
#---

# C source file template:
#   $1  module name
#   $2  filename
#   $3  dependencies
define rule-c-source
build/$1_$2.o: src/$1/$2 $3
	$(cc) -c $$< -o $$@ $(cflags) -I src/$1
endef

# asm source file template:
#   $1  module name
#   $2  filename
define rule-asm-source
build/$1_$2.o: src/$1/$2
	$(as)  -c $$< -o $$@
endef



#---
#	Building.
#---

# Generic rules

all: build $(target-std) $(target-lib) $(target-g1a)
	@echo "[ \033[32;1mOK\033[0m ] All done!"

build:
	mkdir -p $@

$(target-std): $(obj-std)
	$(ar) rcs $@ $^
	@echo "[ \033[32;1mOK\033[0m ] libc: `stat -c %s $@` bytes"

$(target-lib): $(target-std) $(obj-lib)
	$(ar) rcs $@ $(obj-lib)
	@echo "[ \033[32;1mOK\033[0m ] libgint: `stat -c %s $@` bytes"

$(target-g1a): $(target-std) $(target-lib) $(demo-obj)
	$(cc) -o $(demo-elf) $(cflags) -T $(demo-ld) $(demo-obj) $(demo-libs)
	$(ob) -R .comment -R .bss -O binary $(demo-elf) $(demo-bin)
	$(wr) $(demo-bin) -o $@ -i $(demo-icon)
	@echo "[ \033[32;1mOK\033[0m ] demo app: `stat -c %s $@` bytes"

# Automated rules

$(foreach mod,$(modules), \
	$(foreach source,$(mod-$(mod)-c), $(eval \
		$(call rule-c-source,$(mod),$(source),$(hdr-dep)))) \
	$(foreach source,$(mod-$(mod)-asm), $(eval \
		$(call rule-asm-source,$(mod),$(source)))) \
)

# Specific rules

# This one should not be optimized. It makes __attribute__((interrupt_handler))
# buggy... maybe. Anyway there's a bug in this file.
build/core_gint.c.o: src/core/gint.c $(mod-core-dep)
	$(cc) -c $< -o $@ $(cflags) -I src/core -O0

build/display_font_system.bmp.o: src/display/font_system.bmp
	fxconv $< -o $@ --font -n gint_font_system

# Demo application

build/demo_%.c.o: demo/%.c $(hdr-dep) $(demo-dep)
	$(cc) -c $< -o $@ $(cflags)

build/demo_font_%.bmp.o: demo/resources/font_%.bmp
	fxconv $< -o $@ --font -n $(patsubst demo/resources/%.bmp,res_%,$<)

build/demo_%.bmp.o: demo/resources/%.bmp
	fxconv $< -o $@ -n $(patsubst demo/resources/%.bmp,res_%,$<)



#---
#	Cleaning and others.
#---

clean:
	@ rm -rf build/*

mrproper: clean
	@ rm -f $(target-g1a) $(target-lib) $(target-std)
	@ rm -rf build

distclean: mrproper

install:
	CasioUsbUploader -f $(target-g1a) -w -l 1


.PHONY: all clean mrproper distclean
