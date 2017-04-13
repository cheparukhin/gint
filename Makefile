#! /usr/bin/make -f
#
#	gint project Makefile
#
#---

include Makefile.cfg

#---
#	Project variables
#---

# Modules
modules-gint	= bopti clock core display events gray keyboard mmu rtc \
		  screen tales timer
modules-libc	= ctype setjmp stdio stdlib string time

# Targets
target-lib	= libgint.a
target-std	= libc.a
target-g1a	= gintdemo.g1a
target-dbg	= gintdbg.g1a

# Tools
cc		= sh3eb-elf-gcc
as		= sh3eb-elf-as
ar		= sh3eb-elf-ar
ob		= sh3eb-elf-objcopy
wr		= g1a-wrapper

# Flags for gint
lib-cflags	= -m3 -mb -nostdlib -I include -ffreestanding -std=c11 -Os \
		  -Wall -Wextra @gcc.cfg

# Demo application (could be done better)
demo-src	= $(notdir $(wildcard demo/*.[cs]))
demo-dep	= $(wildcard demo/*.h)
demo-icon	= demo/icon.bmp
demo-res	= $(notdir $(wildcard demo/resources/*))
demo-obj	= $(patsubst %,build/demo_%.o,$(demo-src) $(demo-res))
demo-elf	= build/gintdemo.elf
demo-bin	= build/gintdemo.bin
demo-ldflags	= $(demo-cflags) -T demo/gintdemo.ld -L. -lgint -lc -lgcc
demo-cflags	= -m3 -mb -nostdlib -I include -ffreestanding -std=c11 -Os \
		  -Wall -Wextra

# Debugger application (displays past diagnostics without running gint)
debug-src	= $(notdir $(wildcard debug/*.[cs]))
debug-dep	= $(wildcard debug/*.h)
debug-icon	= debug/icon.bmp
debug-obj	= $(debug-src:%=build/debug_%.o)
debug-elf	= build/gintdbg.elf
debug-bin	= build/gintdbg.bin
debug-ldflags	= $(debug-cflags) -T debug/addin.ld -L debug -lfx -lgcc
debug-cflags	= -m3 -mb -nostdlib -ffreestanding -I debug/include -I \
		  include -std=c11 -Os -Wall -Wextra

# Specific objects
obj-lib-spec	= build/display_font_system.bmp.o
obj-std-spec	=

# Configuration files
config		= gcc.cfg
ifeq ($(wildcard $(config)),)
$(error "Configuration files are missing. Did you ./configure ?")
endif

# Target folder
folder		:= $(shell fxsdk --folder)
ifndef folder
$(error "Could not get the fxSDK storage folder. Did you install fxSDK \
properly?")
endif



#---
#	Automatic variables
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

# Dependencies.
hdr-dep = $(wildcard include/*.h include/*/*.h)



#---
#	Rule templates
#---

# C source file template:
#   $1  module name
#   $2  filename
#   $3  dependencies
define rule-c-source
build/$1_$2.o: src/$1/$2 $3 $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m cc $$<\n')
	$(if $(VERBOSE),,@) $(cc) -c $$< -o $$@ $(lib-cflags)
endef

# Asm source file template:
#   $1  module name
#   $2  filename
define rule-asm-source
build/$1_$2.o: src/$1/$2 $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m as $$<\n')
	$(if $(VERBOSE),,@) $(as)  -c $$< -o $$@
endef



#---
#	Version management
#---

# Retrieve version information.
version_string	:= $(shell cat version | sed 's/[-.]/ /g')
version_type	:= $(word 1,$(version_string))
version_major	:= $(word 2,$(version_string))
version_minor	:= $(word 3,$(version_string))
version_build	:= $(word 4,$(version_string))

# Bump build number and make up the new version integer.
version_build	:= $(shell echo $$(($(version_build) + 1)))
version_letter	:= $(shell echo -n $(version_type) | sed -r 's/^(.).*/\1/')
version_symbol	:= $(shell printf '0x%02x%01x%01x%04x' "'$(version_letter)'" \
		$(version_major) $(version_minor) $(version_build))

# Tell the linker to define the version symbol.
ldflags += -Wl,--defsym,GINT_VERSION=$(version_symbol)



#---
#	Building
#---

# Generic rules

all-lib: $(config) $(target-std) $(target-lib)

all: $(config) $(target-std) $(target-lib) $(target-g1a) $(target-dbg)

build:
	$(if $(VERBOSE),,@ printf '\e[35;1mdir \u00bb\e[0m mkdir $@\n')
	$(if $(VERBOSE),,@) mkdir -p $@

version: $(obj-std) $(obj-lib)
	@ echo '$(version_type)-$(version_major).$(version_minor)-$(version_build)' > $@


$(obj-std) $(obj-lib) $(demo-obj): | build

$(target-std): $(obj-std) version
	$(if $(VERBOSE),,@ printf '\e[35;1mlib \u00bb\e[0m ar $@\n')
	$(if $(VERBOSE),,@) $(ar) rcs $@ $(obj-std)
	@ printf '\e[32;1mmsg \u00bb\e[0m Succesfully built libc ('
	@ printf $$(stat -c %s $@)
	@ printf ' bytes)\n\n'

$(target-lib): $(config) $(target-std) $(obj-lib) version
	$(if $(VERBOSE),,@ printf '\e[35;1mlib \u00bb\e[0m ar $@\n')
	$(if $(VERBOSE),,@) $(ar) rcs $@ $(obj-lib)
	@ printf '\e[32;1mmsg \u00bb\e[0m Succesfully built libgint ('
	@ printf $$(stat -c %s $@)
	@ printf ' bytes)\n\n'

$(target-g1a): $(config) $(target-std) $(target-lib) $(demo-obj)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m ld -o $(demo-elf)\n')
	$(if $(VERBOSE),,@) $(cc) -o $(demo-elf) $(demo-obj) $(demo-ldflags)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m objcopy -o $(demo-bin)\n')
	$(if $(VERBOSE),,@) $(ob) -R .comment -R .bss -O binary $(demo-elf) $(demo-bin)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m g1a-wrapper -o $@\n')
	$(if $(VERBOSE),,@) $(wr) $(demo-bin) -o $@ -i $(demo-icon)
	@ printf '\e[32;1mmsg \u00bb\e[0m Succesfully built demo application ('
	@ printf $$(stat -c %s $@)
	@ printf ' bytes)\n\n'

$(target-dbg): $(config) $(debug-obj)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m ld -o $(debug-elf)\n')
	$(if $(VERBOSE),,@) $(cc) -o $(debug-elf) $(debug-obj) $(debug-ldflags)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m objcopy -o $(debug-bin)\n')
	$(if $(VERBOSE),,@) $(ob) -R .comment -R .bss -O binary $(debug-elf) $(debug-bin)
	$(if $(VERBOSE),,@ printf '\e[35;1mexe \u00bb\e[0m g1a-wrapper -o $@\n')
	$(if $(VERBOSE),,@) $(wr) $(debug-bin) -o $@ -i $(debug-icon)
	@ printf '\e[32;1mmsg \u00bb\e[0m Succesfully built debug application ('
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

# Optimizing this one makes the interrupt handler raise illegal slot exception
# on rte; lds.l @r15+, mach. This is totally weird but I haven't understood
# why for now.

build/display_font_%.bmp.o: src/display/font_%.bmp
	$(if $(VERBOSE),,@ printf '\e[36;1mres \u00bb\e[0m fxconv $<\n')
	$(if $(VERBOSE),,@) fxconv $< -o $@ --font -n $(<:src/display/font_%.bmp=gint_font_%)

# Demo application

build/demo_%.c.o: demo/%.c $(hdr-dep) $(demo-dep) $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m cc $<\n')
	$(if $(VERBOSE),,@) $(cc) -c $< -o $@ $(demo-cflags)

build/demo_font_%.bmp.o: demo/resources/font_%.bmp
	$(if $(VERBOSE),,@ printf '\e[36;1mres \u00bb\e[0m fxconv $<\n')
	$(if $(VERBOSE),,@) fxconv $< -o $@ --font -n $(patsubst demo/resources/%.bmp,res_%,$<)

build/demo_%.bmp.o: demo/resources/%.bmp
	$(if $(VERBOSE),,@ printf '\e[36;1mres \u00bb\e[0m fxconv $<\n')
	$(if $(VERBOSE),,@) fxconv $< -o $@ -n $(patsubst demo/resources/%.bmp,res_%,$<)

# Debug application

build/debug_%.s.o: debug/%.s $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m as $<\n')
	$(if $(VERBOSE),,@) $(as) -c $< -o $@

build/debug_%.c.o: debug/%.c $(hdr-dep) $(debug-dep) $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1msrc \u00bb\e[0m cc $<\n')
	$(if $(VERBOSE),,@) $(cc) -c $< -o $@ $(debug-cflags)



#---
#	Cleaning and install
#---

clean:
	@ rm -rf build

mrproper: clean
	@ rm -f $(target-g1a) $(target-lib) $(target-std)
	@ rm -f $(config)

distclean: mrproper

install: $(target-std) $(target-lib)
	mkdir -p $(folder)
	install -m 644 $^ $(folder)
	install -m 644 -T demo/gintdemo.ld $(folder)/linker.ld
	mkdir -p $(folder)/gint
	install -m 644 include/*.h $(folder)/gint
ifdef config_ext
	install -m 644 include/extended/*.h $(folder)/gint
endif
	@ printf '\e[32;1mmsg \u00bb\e[0m All installed!\n'

install-demo: all
	p7 send -f $(target-g1a)
install-debug: all
	p7 send -f $(target-dbg)

.PHONY: all-lib all help
.PHONY: clean mrproper distclean
.PHONY: install install-demo install-debug
