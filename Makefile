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

cf-fx	:= -m3 -mb -ffreestanding -nostdlib -Wall -Wextra -std=c11 -Os \
	   -fstrict-volatile-bitfields -I include $(cfg_macros)
cf-cg	:= -m4 -mb -ffreestanding -nostdlib -Wall -Wextra -std=c11 -Os \
	   -fstrict-volatile-bitfields -I include $(cfg_macros)

# On fx9860g, use the sh3eb-elf toolchain; on fxcg50, prefer sh4eb-nofpu-elf
toolchain := $(if $(filter $(cfg_target),fx),sh3eb-elf,sh4eb-nofpu-elf)

# Compiler flags, assembler flags, dependency generation, archiving
cflags	:= $(cf-$(cfg_target))
sflags	:= $(cfg_macros)
dflags	 = -MMD -MT $@ -MF $(@:.o=.d) -MP
arflags	:=

#
#  File listings
#

# Target file (cfg_target is either "fx" or "cg")
target	:= bin/libgint-$(cfg_target).a

# Automatic names for object and dependency files
src2obj	 = build/$(1:src/%=%).o
src2dep	 = build/$(1:src/%=%).d

# Source files
src	:= $(shell find src -name '*.[csS]')
src_obj	:= $(foreach s,$(src),$(call src2obj,$s))

# Files with special handling
# spe	:= build/version.o src/display/font.bmp
spe	:=
spe_obj	:= build/version.o $(foreach s,$(spe),$(call src2obj,$s))

# All object files
obj	:= $(src_obj) $(spe_obj)

# Version file
version	:= config/version

#
#  Toolchain
#

gcc	= $(toolchain)-gcc
as	= $(toolchain)-as
ld	= $(toolchain)-ld
ar	= $(toolchain)-ar
objcopy	= $(toolchain)-objcopy
conv	= fxconv


#
#	Version management
#

# Retrieve version information
v_file	= $(shell cat $(version) | sed 's/[-.]/ /g')
v_type	= $(word 1,$(v_file))
v_major	= $(word 2,$(v_file))
v_minor	= $(word 3,$(v_file))
v_build	= $(word 4,$(v_file))

# Create the current version symbol and the new version integer
v_newbuild	= $(shell echo $$(($(v_build) + 1)))
v_letter	= $(shell echo -n $(v_type) | sed -r 's/^(.).*/\1/')
v_symbol	= $(shell printf '0x%02x%01x%01x%04x' "'$(v_letter)'" \
		  $(v_major) $(v_minor) $(v_build))

#
#  Build rules
#

all: $(target)

$(target): $(obj) $(version) | $(dir $(target))
	$(call cmd_l,ar,$@) $(ar) -crs $(arflags) $@ $(obj)

# Assembler sources
build/%.s.o: src/%.s build/%.s.d
	@ mkdir -p $(dir $@)
	$(call cmd_b,as,$<) $(gcc) -c $< -o $@ $(sflags)
build/%.S.o: src/%.S build/%.S.d
	@ mkdir -p $(dir $@)
	$(call cmd_b,as,$<) $(gcc) -c $< -o $@ $(sflags)

# C sources
build/%.c.o: src/%.c build/%.c.d $(cfgdep)
	@ mkdir -p $(dir $@)
	$(call cmd_b,gcc,$<) $(gcc) -c $< -o $@ $(dflags) $(cflags)

# Special files
$(call src2obj,src/display/font.bmp): src/display/font.bmp
	@ mkdir -p $(dir $@)
	$(call cmd_m,fxconv,$<) $(conv) -font $< -n gint_font -o $@

# Version symbol. ld generates a .stack section for unknown reasons; I fall
# back to removing it afterwards.
build/version.o:
	@ mkdir -p $(dir $@)
	@ echo "_GINT_VERSION = $(v_symbol);" > $@.txt
	$(call cmd_b,ld,$@) $(ld) -r -R $@.txt -o $@

#
#  Cleaning
#

clean:
	@ rm -rf build/
distclean: clean
	@ rm -rf bin/
	@ rm -f $(config)

#
#  Utilities
#

$(version): $(src) $(wildcard include/**/*)
	@ echo '$(v_type)-$(v_major).$(v_minor)-$(v_newbuild)' > $@

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
