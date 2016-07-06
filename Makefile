## Temporary things there

# C source file template:
#   $1  module name
#   $2  file base name
define rule-c-source
build/$1_$2.c.o: src/$1/$2.c
	$(cc) $(cflags) -c $$< -o $$@
endef

$(eval $(call rule-c-source,display,area))
