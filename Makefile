#! /usr/bin/make -f
#
#	gint project Makefile
#
#---

#
#  Build configuration
#

# Some rules don't care if config files are missing (eg. clean, etc)
cfg	:= config/Makefile.cfg
-include $(cfg)

# Those that do may use this special dependency
cfgdep	:= $(if $(shell [ -f $(cfg) ] || echo n),CFGDEP,$(cfg))

# Compiler flags, assembler flags, dependency generation, archiving
cflags	:= -m3 -mb -ffreestanding -nostdlib -Wall -Wextra -std=c11 -O2 \
	   -I include $(cfg_defs)
dflags	 = -MMD -MT $@ -MF $(@:.o=.d) -MP
arflags	:=

#
#  File listings
#

# Target file
target	:= bin/libgint.a

# Automatic names for object and dependency files
src2obj	 = build/$(1:src/%=%).o
src2dep	 = build/$(1:src/%=%).d

# Source files
src	:= $(shell find src -name '*.[cs]')
src_obj	:= $(foreach s,$(src),$(call src2obj,$s))

# Files with special handling
spe	:= src/display/font.bmp
spe_obj	:= $(foreach s,$(spe),$(call src2obj,$s))

# All object files
obj	:= $(src_obj) $(spe_obj)

#
#  Toolchain
#

gcc	= sh3eb-elf-gcc
as	= sh3eb-elf-as
ld	= sh3eb-elf-ld
ar	= sh3eb-elf-ar
conv	= fxconv

#
#  Build rules
#

all: $(target)

$(target): $(obj) | $(dir $(target))
	$(call cmd_l,ar,$@) $(ar) -crs $(arflags) $@ $(obj)

# Assembler sources
build/%.s.o: src/%.s build/%.s.d
	@ mkdir -p $(dir $@)
	$(call cmd_b,as,$<) $(as) -c $< -o $@ $(sflags)

# C sources
build/%.c.o: src/%.c build/%.c.d $(cfgdep)
	@ mkdir -p $(dir $@)
	$(call cmd_b,gcc,$<) $(gcc) -c $< -o $@ $(dflags) $(cflags)

# Special files
$(call src2obj,src/display/font.bmp): src/display/font.bmp
	@ mkdir -p $(dir $@)
	$(call cmd_m,fxconv,$<) $(conv) -font $< -n gint_font -o $@

#
#  Cleaning
#

clean:
	@ rm -rf build/
distclean: clean
	@ rm -rf bin/
	@ rm -f $(cfg)

#
#  Utilities
#

# Evaluated when a rule requires the configuration file but it doesn't exist
CFGDEP:
	@ echo "Configuration file $(cfg) is missing. Have you configured?"
	@ echo "See \`./configure --help\` for details."
	@ false

# Make directories: make conveniently leaves a '/' at the end of $(dir ...)
%/:
	@ mkdir -p $@
# Don't try to unlink directories once they're built (that wouldn't work =p)
.PRECIOUS: %/

# Dependency information
-include $(shell [ -d build ] && find build -name *.d)
build/%.d: ;
.PRECIOUS: build/%.d

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
