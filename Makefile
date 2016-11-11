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
modules-gint	= core clock keyboard mmu mpu rtc screen timer \
		  bopti display gray tales events
modules-libc	= setjmp string stdio stdlib time

# Targets
target-g1a	= gintdemo.g1a
target-lib	= libgint.a
target-std	= libc.a

# Tools
cc		= sh3eb-elf-gcc
as		= sh3eb-elf-as
ar		= sh3eb-elf-ar
ob		= sh3eb-elf-objcopy
wr		= g1a-wrapper

# Flags
cflags		= -m3 -mb -nostdlib -I include -ffreestanding -std=c11 -Os \
		  -W -Wall -Wextra -pedantic @gcc.cfg

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

# Configuration files
config		= gcc.cfg

#---
#	Automatic variables.
#---

# Modules are subfolders of src/.
modules = $(modules-gint) $(modules-libc)

define n
# This is a newline character.

endef

ifeq ("$(wildcard $(config))","")
$(error "Configuration files are missing. Did you ./configure?")
endif

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

#ifndef VERBOSE
#$(note "default full log")
#VERBOSE = 
#endif

# C source file template:
#   $1  module name
#   $2  filename
#   $3  dependencies
define rule-c-source
build/$1_$2.o: src/$1/$2 $3 $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m cc $$<\n')
	$(if $(VERBOSE),,@) $(cc) -c $$< -o $$@ $(cflags) -I src/$1
endef

# asm source file template:
#   $1  module name
#   $2  filename
define rule-asm-source
build/$1_$2.o: src/$1/$2 $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m as $$<\n')
	$(if $(VERBOSE),,@) $(as)  -c $$< -o $$@
endef



#---
#	Building.
#---

# Generic rules

all: $(config) $(target-std) $(target-lib) $(target-g1a)
	@ printf '\e[32;1mmsg \u00bb\e[0m All done!\n'

build:
	$(if $(VERBOSE),,@ printf '\e[35;1mdir \u00bb\e[0m mkdir $@\n')
	$(if $(VERBOSE),,@) mkdir -p $@

$(obj-std) $(obj-lib) $(demo-obj): | build

$(target-std): $(config) $(obj-std)
	$(if $(VERBOSE),,@ printf '\e[35;1mlib \u00bb\e[0m ar $@\n')
	$(if $(VERBOSE),,@) $(ar) rcs $@ $(obj-std)
	@ printf '\e[32;1mmsg \u00bb\e[0m Succesfully built libc ('
	@ printf $$(stat -c %s $@)
	@ printf ' bytes)\n\n'

$(target-lib): $(config) $(target-std) $(obj-lib)
	$(if $(VERBOSE),,@ printf '\e[35;1mlib \u00bb\e[0m ar $@\n')
	$(if $(VERBOSE),,@) $(ar) rcs $@ $(obj-lib)
	@ printf '\e[32;1mmsg \u00bb\e[0m Succesfully built libgint ('
	@ printf $$(stat -c %s $@)
	@ printf ' bytes)\n\n'

$(target-g1a): $(config) $(target-std) $(target-lib) $(demo-obj)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m ld -o $(demo-elf)\n')
	$(if $(VERBOSE),,@) $(cc) -o $(demo-elf) $(cflags) -T $(demo-ld) $(demo-obj) $(demo-libs)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m objcopy -o $(demo-bin)\n')
	$(if $(VERBOSE),,@) $(ob) -R .comment -R .bss -O binary $(demo-elf) $(demo-bin)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m g1a-wrapper -o $@\n')
	$(if $(VERBOSE),,@) $(wr) $(demo-bin) -o $@ -i $(demo-icon)
	@ printf '\e[32;1mmsg \u00bb\e[0m Succesfully built demo application ('
	@ printf $$(stat -c %s $@)
	@ printf ' bytes)\n\n'

# Automated rules

$(foreach mod,$(modules), \
	$(foreach source,$(mod-$(mod)-c), $(eval \
		$(call rule-c-source,$(mod),$(source),$(hdr-dep)))) \
	$(foreach source,$(mod-$(mod)-asm), $(eval \
		$(call rule-asm-source,$(mod),$(source)))) \
)

# Specific rules

# This one should not be optimized. It makes __attribute__((interrupt_handler))
# buggy... maybe. Anyway there's some bug in this file that I can't fix now.
build/core_gint.c.o: src/core/gint.c $(mod-core-dep) $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m cc $<\n')
	$(if $(VERBOSE),,@) $(cc) -c $< -o $@ $(cflags) -I src/core -O0

build/display_font_system.bmp.o: src/display/font_system.bmp
	$(if $(VERBOSE),,@ printf '\e[36;1mres \u00bb\e[0m fxconv $<\n')
	$(if $(VERBOSE),,@) fxconv $< -o $@ --font -n gint_font_system

# Demo application

build/demo_%.c.o: demo/%.c $(hdr-dep) $(demo-dep) $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m cc $<\n')
	$(if $(VERBOSE),,@) $(cc) -c $< -o $@ $(cflags)

build/demo_font_%.bmp.o: demo/resources/font_%.bmp
	$(if $(VERBOSE),,@ printf '\e[36;1mres \u00bb\e[0m fxconv $<\n')
	$(if $(VERBOSE),,@) fxconv $< -o $@ --font -n $(patsubst demo/resources/%.bmp,res_%,$<)

build/demo_%.bmp.o: demo/resources/%.bmp
	$(if $(VERBOSE),,@ printf '\e[36;1mres \u00bb\e[0m fxconv $<\n')
	$(if $(VERBOSE),,@) fxconv $< -o $@ -n $(patsubst demo/resources/%.bmp,res_%,$<)



#---
#	Cleaning and others.
#---

clean:
	@ rm -rf build

mrproper: clean
	@ rm -f $(target-g1a) $(target-lib) $(target-std)
	@ rm -f $(config)

distclean: mrproper

install:
	p7 send -f $(target-g1a)


.PHONY: all clean mrproper distclean install help
