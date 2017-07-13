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
modules-gint	= bopti clock core display events gray init keyboard mmu rtc \
		  screen tales timer
modules-libc	= ctype math setjmp stdio stdlib string time

# Targets
target-lib	= libgint.a
target-std	= libc.a
target-g1a	= gintdemo.g1a

# Tools
cc		= sh3eb-elf-gcc
ld		= sh3eb-elf-ld
as		= sh3eb-elf-as
ar		= sh3eb-elf-ar
ob		= sh3eb-elf-objcopy
wr		= g1a-wrapper

# Flags for gint
lib-cflags	= -m3 -mb -nostdlib -I include -ffreestanding -std=c11 -Os \
		  -Wall -Wextra @gcc.cfg -g0

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

# Specific objects
obj-std-spec	=
obj-lib-spec	= build/display_font_system.bmp.o build/version.o

# Configuration files, require them only if we're not cleaning
config		= gcc.cfg
ifeq "$(filter clean mrproper distclean,$(MAKECMDGOALS))" ""
ifeq ($(wildcard $(config)),)
$(error "Configuration files are missing. Did you ./configure ?")
endif
endif

# Target folder, require it only if we want to install
ifneq "$(filter install,$(MAKECMDGOALS))" ""
folder		:= $(shell fxsdk --folder)
ifndef folder
$(error "Could not get the fxSDK storage folder. Did you install fxSDK \
properly?")
endif
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
	$(if $(VERBOSE),,@ printf '\e[34;1m    gcc\e[0m $$<\n')
	$(if $(VERBOSE),,@) $(cc) -c $$< -o $$@ $(lib-cflags)
endef

# Asm source file template:
#   $1  module name
#   $2  filename
define rule-asm-source
build/$1_$2.o: src/$1/$2 $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1m     as\e[0m $$<\n')
	$(if $(VERBOSE),,@) $(as)  -c $$< -o $$@
endef



#---
#	Version management
#---

# Retrieve version information.
version_string	= $(shell cat version | sed 's/[-.]/ /g')
version_type	= $(word 1,$(version_string))
version_major	= $(word 2,$(version_string))
version_minor	= $(word 3,$(version_string))
version_build	= $(word 4,$(version_string))

# Make up the new version integer.
version_build_n	= $(shell echo $$(($(version_build) + 1)))
version_letter	= $(shell echo -n $(version_type) | sed -r 's/^(.).*/\1/')
version_symbol	= $(shell printf '0x%02x%01x%01x%04x' "'$(version_letter)'" \
		$(version_major) $(version_minor) $(version_build))



#---
#	Building
#---

# Generic rules

all-lib: $(config) $(target-std) $(target-lib)

all: $(config) $(target-std) $(target-lib) $(target-g1a)

build:
	$(if $(VERBOSE),,@ printf '\e[35;1m  mkdir\e[0m $@\n')
	$(if $(VERBOSE),,@) mkdir -p $@

version: src include
	@ echo '$(version_type)-$(version_major).$(version_minor)-$(version_build_n)' > $@


$(obj-std) $(obj-lib) $(demo-obj): | build

$(target-std): $(obj-std) version
	$(if $(VERBOSE),,@ printf '\e[35;1m     ar\e[0m ar $@\n')
	$(if $(VERBOSE),,@) $(ar) rcs $@ $(obj-std)
	@ printf '\n\e[32;1m\u00bb\e[0m Succesfully built libc ('
	@ printf $$(stat -c %s $@)
	@ printf ' bytes)\n\n'

$(target-lib): $(config) $(target-std) $(obj-lib) version
	$(if $(VERBOSE),,@ printf '\e[35;1m     ar\e[0m $@\n')
	$(if $(VERBOSE),,@) $(ar) rcs $@ $(obj-lib)
	@ printf '\n\e[32;1m\u00bb\e[0m Succesfully built libgint ('
	@ printf $$(stat -c %s $@)
	@ printf ' bytes)\n\n'

$(target-g1a): $(config) $(target-std) $(target-lib) $(demo-obj)
	$(if $(VERBOSE),,@ printf '\e[35;1m     ld\e[0m $(demo-elf)\n')
	$(if $(VERBOSE),,@) $(cc) -o $(demo-elf) $(demo-obj) $(demo-ldflags)
	$(if $(VERBOSE),,@ printf '\e[35;1mobjcopy\e[0m $(demo-bin)\n')
	$(if $(VERBOSE),,@) $(ob) -R .comment -R .bss -O binary $(demo-elf) $(demo-bin)
	$(if $(VERBOSE),,@ printf '\e[35;1m  fxg1a\e[0m $@\n')
	$(if $(VERBOSE),,@) $(wr) $(demo-bin) -o $@ -i $(demo-icon)
	@ printf '\n\e[32;1m\u00bb\e[0m Succesfully built demo application ('
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

# Define the version symbol in a specific object file. ld generates a .stack
# section on sh3eb-elf, which it didn't on x86_64. I don't understand the
# details of why, so I just fall back to removing it afterwards.
build/version.o:
	@ echo "_GINT_VERSION = $(version_symbol);" > $@.txt
	$(if $(VERBOSE),,@ printf '\e[35;1m     ld\e[0m $@\n')
	$(if $(VERBOSE),,@) $(ld) -r -R $@.txt -o $@
	$(if $(VERBOSE),,@) $(ob) -R .stack $@ $@

build/display_font_%.bmp.o: src/display/font_%.bmp
	$(if $(VERBOSE),,@ printf '\e[30;1m fxconv\e[0m -font $<\n')
	$(if $(VERBOSE),,@) fxconv -font $< -o $@ -font -n $(<:src/display/font_%.bmp=gint_font_%)

# Demo application

build/demo_%.c.o: demo/%.c $(hdr-dep) $(demo-dep) $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1m    gcc\e[0m $<\n')
	$(if $(VERBOSE),,@) $(cc) -c $< -o $@ $(demo-cflags)

build/demo_%.s.o: demo/%.s $(config)
	$(if $(VERBOSE),,@ printf '\e[34;1m     as\e[0m $<\n')
	$(if $(VERBOSE),,@) $(as) -c $< -o $@

build/demo_font_%.bmp.o: demo/resources/font_%.bmp
	$(if $(VERBOSE),,@ printf '\e[30;1m fxconv\e[0m -font $<\n')
	$(if $(VERBOSE),,@) fxconv -font $< -o $@ -n $(patsubst demo/resources/%.bmp,res_%,$<)

build/demo_%.bmp.o: demo/resources/%.bmp
	$(if $(VERBOSE),,@ printf '\e[30;1m fxconv\e[0m -image $<\n')
	$(if $(VERBOSE),,@) fxconv -image $< -o $@ -n $(patsubst demo/resources/%.bmp,res_%,$<)



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
	$(if $(VERBOSE),,@ printf '\e[35;1m  mkdir\e[0m $(folder)\n')
	$(if $(VERBOSE),,@) mkdir -p $(folder)

	$(if $(VERBOSE),,@ printf '\e[33;1minstall\e[0m 644 $^\n')
	$(if $(VERBOSE),,@) install -m 644 $^ $(folder)
	$(if $(VERBOSE),,@ printf '\e[33;1minstall\e[0m 644 demo/gintdemo.ld\n')
	$(if $(VERBOSE),,@) install -m 644 -T demo/gintdemo.ld $(folder)/linker.ld

	$(if $(VERBOSE),,@ printf '\e[35;1m  mkdir\e[0m $(folder)/gint/modules\n')
	$(if $(VERBOSE),,@) mkdir -p $(folder)/gint/modules

	$(if $(VERBOSE),,@ printf '\e[33;1minstall\e[0m 644 include/**.h\n')
	$(if $(VERBOSE),,@) install -m 644 include/*.h $(folder)/gint
	$(if $(VERBOSE),,@) install -m 644 include/modules/*.h $(folder)/gint/modules
ifdef config_ext
	$(if $(VERBOSE),,@) install -m 644 include/extended/*.h $(folder)/gint
endif
	@ printf "\n\033[32;1m\u00bb\033[0m Successfully installed gint\n\n"

install-demo: all
	p7 send -f $(target-g1a)

.PHONY: all-lib all help
.PHONY: clean mrproper distclean
.PHONY: install install-demo
