VERSION_MAJOR		= 0
VERSION_MINOR		= 1
PATCHLEVEL		= 0
EXTRAVERSION		= 0
NAME			= uld-fdpic

.SECONDEXPANSION:

TARGET ?= stm32f103xb

ULD_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SCRIPTS_DIR = $(abspath $(ULD_DIR)/scripts)
FDPIC_BIN_PATH ?= $(abspath $(ULD_DIR)/../toolchain-install/bin)

GCC_ARM_EMBEDDED ?= 0
ifeq ($(GCC_ARM_EMBEDDED),0)
CROSS_COMPILE = arm-none-eabifdpic-
PATH := $(FDPIC_BIN_PATH):$(PATH)
NO_FDPIC = -mno-fdpic
else
CROSS_COMPILE = arm-none-eabi-
NO_FDPIC =
endif

ADDR2LINE		= $(CROSS_COMPILE)addr2line
AR			= $(CROSS_COMPILE)ar
AS			= $(CROSS_COMPILE)as
CC			= $(CROSS_COMPILE)gcc
CPP			= $(CROSS_COMPILE)cpp
CXX			= $(CROSS_COMPILE)g++
ELFEDIT			= $(CROSS_COMPILE)elfedit
GCC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
NM			= $(CROSS_COMPILE)nm
OBJCOPY			= $(CROSS_COMPILE)objcopy
OBJDUMP 		= $(CROSS_COMPILE)objdump
RANLIB			= $(CROSS_COMPILE)ranlib
READELF 		= $(CROSS_COMPILE)readelf
SIZE			= $(CROSS_COMPILE)size
STRINGS			= $(CROSS_COMPILE)strings
STRIP			= $(CROSS_COMPILE)strip

AWK			= awk
CRC32			= crc32
DD			= dd
MKDIR			= mkdir
PERL			= perl
PYTHON			= /usr/bin/env python
RM			= rm

GEN_ULD_FILES_SCR = $(SCRIPTS_DIR)/gen-uld-files.py
PATCH_FST_CRC_SCR = $(SCRIPTS_DIR)/patch-fst-crc.sh
PATCH_ULD_ELF_SCR = $(SCRIPTS_DIR)/patch-uld-elf.py

INCLUDE_DIRS = cmsis soc hal
EXTRA_INCLUDE_DIRS = src
INCLUDE_FILES =

SPECS ?= nosys.specs

CLEAN_DIRS = $(OBJECT_DIR) $(BUILD_OUTPUT) include/generated

OPTLVL ?=
ifneq ($(OPTLVL),)
OPTLVL := -O$(patsubst -O%,%,$(OPTLVL))
ifeq ($(findstring $(OPTLVL),-O0 -O1 -O2 -Os),)
$(error invalid optlvl $(OPTLVL))
endif
endif

# Linker flags need march/mthumb for multilib to work correctly.
# See option --print-multi-lib.
FLAGS_COMMON = -g \
	$(if $(SPECS),-specs=$(SPECS),) \
	-march=$(TARGET_ARCH) \
	-mcpu=$(TARGET_CPU) \
	-mthumb $(OPTLVL)

CFLAGS_COMMON = -std=gnu11 \
	-Wall -Wextra -Wstrict-prototypes -Wno-unused-parameter \
	-fdata-sections \
	-ffunction-sections

CFLAGS_SO = -fvisibility=hidden

WPTRARITH ?= 0
ifneq ($(WPTRARITH),0)
CFLAGS_COMMON += -Wpointer-arith
endif

WNOERROR ?= 0
ifeq ($(WNOERROR),0)
CFLAGS_COMMON += -Werror
else
# Leave select warning as errors enabled.
CFLAGS_COMMON += -Werror=implicit-function-declaration
endif

CFLAGS_CORTEX_COMMON = \
	-fsingle-precision-constant \
	-mabi=aapcs \
	-mlittle-endian \
	-mthumb-interwork

CFLAGS_CORTEX_M4_ARCH		= armv7e-m
CFLAGS_CORTEX_M4_FP_HARDABI	= -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS_CORTEX_M4_FP_SOFTABI	= -mfloat-abi=softfp -mfpu=fpv4-sp-d16

CFLAGS_CORTEX_M3_ARCH		= armv7-m

CFLAGS = $(FLAGS_COMMON) $(CFLAGS_COMMON)


LD_COMMON_DIRS = $(bin)
LD_LIB_DIRS = $(LD_COMMON_DIRS)
LD_RPATH_LINK_DIRS = $(LD_COMMON_DIRS)
LIBS =

LDFLAGS = $(FLAGS_COMMON) -Wl,--warn-common,--no-undefined,--fatal-warnings
LDFLAGS += -Wl,--gc-sections -nostdlib
LDFLAGS_SO = -shared
LDSCRIPT_TYPE ?= qemu
lds = $(1)$(if $(2),_$(2),)$(if $(3),_$(3),).ld
LDSCRIPT = $(call lds,$(LDSCRIPT_BASE),$(LDSCRIPT_TYPE),$(LDSCRIPT_SUBTYPE))


STRIP_FLAGS_ELF = --strip-unneeded
STRIP_FLAGS_SO = --strip-unneeded


var_ge_str = $(findstring $(shell test $(1) -ge $(2) > /dev/null 2>&1; \
	echo $$?),0 2)

GCCDEV ?= 0
ifneq ($(call var_ge_str,$(GCCDEV),1),)
SAVE_TEMPS = 1
CFLAGS += -fverbose-asm
ifneq ($(call var_ge_str,$(GCCDEV),2),)
# -da broken in gcc 4.7 - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60484
#CFLAGS += -da
CFLAGS += -dA -dp -dP
endif
endif

SAVE_TEMPS ?= 0
ifneq ($(SAVE_TEMPS),0)
CFLAGS += -save-temps=obj
endif

QEMU ?= 1
ifneq ($(QEMU),0)
CFLAGS += -DQEMU
endif

# V >= 1: enable script verbose
# V >= 2: enable script and GCC verbose
# V >= 3 or string: enable script, GCC and LD verbose
# else: normal output
V ?= 1
ifneq ($(call var_ge_str,$(V),1),)
SCR_VERBOSE = --verbose
ifneq ($(call var_ge_str,$(V),2),)
FLAGS_COMMON += -v
ifneq ($(call var_ge_str,$(V),3),)
LDFLAGS += -Wl,--verbose
endif
endif
endif

ifeq ($(TARGET),stm32f103xb)
CFLAGS += $(CFLAGS_CORTEX_COMMON)
TARGET_ARCH = $(CFLAGS_CORTEX_M3_ARCH)
TARGET_CPU = cortex-m3
LDSCRIPT_BASE = stm32f103xb
else
$(error unknown target $(TARGET))
endif


SOURCE_DIR	?= src
OBJECT_DIR	?= obj
BUILD_OUTPUT	?= bin


PHONY := _all
_all: all

export $(exports)


include $(SCRIPTS_DIR)/Kbuild.include
includedir = include
src = $(SOURCE_DIR)
obj = $(OBJECT_DIR)
bin = $(BUILD_OUTPUT)
CFLAGS_INCPATH = -I$(includedir) \
	$(addprefix -I,$(addprefix $(includedir)/,$(INCLUDE_DIRS)) \
	$(EXTRA_INCLUDE_DIRS)) $(addprefix -include ,$(INCLUDE_FILES))
LDFLAGS_LIBPATH = $(addprefix -L,$(LD_LIB_DIRS))
LDFLAGS_LIBPATH += $(addprefix -Wl$(comma)-rpath-link$(comma), \
	$(LD_RPATH_LINK_DIRS))
LDFLAGS_LIBS = $(addprefix -l,$(LIBS))

c_flags = $(strip $(CFLAGS) $(CFLAGS_INCPATH) $(CFLAGS_EXTRA) $(CFLAGS-$@))
a_flags = $(strip $(CFLAGS) $(CFLAGS_INCPATH) $(CFLAGS_EXTRA) $(CFLAGS-$@))
ld_flags = $(strip $(LDFLAGS) $(LDFLAGS_LIBPATH) $(LDFLAGS_EXTRA) \
	-T$(SCRIPTS_DIR)/$(LDSCRIPT) $(LDFLAGS-$@))

target_flags = $(foreach tgt,$(1),$(eval $(2)-$(tgt) += $(3)))
target_cflags = $(call target_flags,$(1),CFLAGS,$(2))
target_ldflags = $(call target_flags,$(1),LDFLAGS,$(2))


mkdir = $(if $(wildcard $(1)),,$(shell echo $(MKDIR) -p $(1));)
objsub = \
	$(patsubst %.c,%.o, \
	$(patsubst %.cpp,%.o, \
	$(patsubst %.cxx,%.o, \
	$(patsubst %.s,%.o, \
	$(patsubst %.S,%.o, $(1))))))
relobj = $(addprefix $$(obj)/,$(1))

dot-arg = $(dir $(1)).$(notdir $(1))
depfile-arg = $(subst $(comma),_,$(dot-arg).d)
depfile-list = $(foreach objfile,$(1),$(call depfile-arg,$(objfile)))

# Renames the elf prereqs for cmd_gen_uld_files
# e.g. $(bin)/foo_strip.elf -> foo.elf=$(bin)/foo_strip.elf to change
# the file name generated by gen-uld-files.py for the fs_table.
gen-uld-files-rename = $(foreach file,$(filter-out $(firstword $^),$^), \
	$(if $(filter %.elf,$(file)), \
	$(file:$(bin)/%_strip.elf=%.elf)=$(file), \
	$(file:$(bin)/%_strip.so=%.so)=$(file)))

# 1: sub dir
# 2: obj dir
obj-dir = $(if $(2),$(2),$(obj))$(if $(1),/$(1),)
# 1: obj-dir
# 2: obj list
relobj-new = $(addprefix $(1)/,$(2))
# make-obj* usage:
# 1: target
# 2: source list
# 3: obj list variable name
# 4: (optional) sub directory from $(obj)
# 5: (optional) directory to override $(obj)
# Note: Split make-obj into multple functions.  The evaluation of
# make-obj_src-to-obj must complete before make can use the computed name
# of the object list (arg 3).
define make-obj_src-to-obj
$(3) += $(call relobj-new,$(call obj-dir,$(4),$(5)),$(call objsub,$(2)))
endef
define make-obj_rules-dep
-include $(call depfile-list,$($(3)))
$(1): obj = $(call obj-dir,$(4),$(5))
endef
define make-obj
$(eval $(call make-obj_src-to-obj,$(1),$(2),$(strip $(3)),$(strip $(4)),$(5)));
$(eval $(call make-obj_rules-dep,$(1),$(2),$(strip $(3)),$(strip $(4)),$(5)));
$(eval $(call imp-rules,$(call obj-dir,$(strip $(4)),$(5))));
$(eval $(if $(5),CLEAN_DIRS += $(5),))
endef


cmd_cc_o_c = $(CC) -Wp,-MD,$(depfile),-MT,$@ $(c_flags) -c -o $@ $<
cmd_cc_o_s = $(CC) -Wp,-MD,$(depfile),-MT,$@ $(a_flags) -c -o $@ $<
cmd_cc_o_S = $(CC) -Wp,-MD,$(depfile),-MT,$@ $(a_flags) -c -o $@ $<
cmd_cc_o_null = $(CC) -Wp,-MD,$(depfile),-MT,$@ $(a_flags) \
	-x assembler-with-cpp -c -o $@ - < /dev/null
cmd_link_elf_o = $(CC) $(ld_flags) -Wl,-Map=$(@:.elf=.map) \
	-o $@ $^ $(LDFLAGS_LIBS)
cmd_link_elf_o_filt = $(CC) $(ld_flags) -Wl,-Map=$(@:.elf=.map) \
	-o $@ $(filter %.o %.a,$^) $(LDFLAGS_LIBS)
cmd_link_so_o = $(CC) $(ld_flags) $(LDFLAGS_SO) -Wl,-Map=$(@:.so=.map) \
	-o $@ $^ $(LDFLAGS_LIBS)

cmd_objc_bin_elf = $(OBJCOPY) -S -O binary $< $@

cmd_objd_lst_elf = $(OBJDUMP) -S $< > $@
cmd_objd_lst_so = $(OBJDUMP) -S $< > $@

cmd_strip_elf_elf = $(STRIP) $(STRIP_FLAGS_ELF) -o $@ $<
cmd_strip_so_so = $(STRIP) $(STRIP_FLAGS_SO) -o $@ $<

cmd_gen_uld_files = OBJCOPY=$(OBJCOPY) $(GEN_ULD_FILES_SCR) \
	--file-path-strip=$(bin)/ $(SCR_VERBOSE) $@ \
	$<$(gen-uld-files-rename)
cmd_patch_uld_elf = OBJCOPY=$(OBJCOPY) READELF=$(READELF) \
	$(PATCH_ULD_ELF_SCR) $(SCR_VERBOSE) $<; \
	touch $@
cmd_objc_uld_gdb_elf = $(OBJCOPY) -R .files $< $@

cmd_mkdir = \
	@set -e; \
	$(if $(2),$(2);,) \
	$(call mkdir,$(@D)) \
	echo "$(cmd_$(1))"; $(cmd_$(1));

cmd_mkdir_dep = \
	$(call cmd_mkdir,$(1),$(RM) -f $(depfile)) \
	printf '%s\n' 'cmd_$@ := $(make-cmd)' >> $(depfile);

if_changed_mkdir_dep = $(if $(strip $(any-prereq) $(arg-check)), \
	$(call cmd_mkdir_dep,$(1)),)


$(bin)/%.bin: $(bin)/%.elf FORCE
	$(call if_changed_mkdir_dep,objc_bin_elf)

$(bin)/%.lst: $(bin)/%.elf FORCE
	$(call if_changed_mkdir_dep,objd_lst_elf)

$(bin)/%_strip.elf: $(bin)/%.elf FORCE
	$(call if_changed_mkdir_dep,strip_elf_elf)

$(bin)/%_gdb.elf: $(bin)/%.elf FORCE
	$(call if_changed_mkdir_dep,objc_uld_gdb_elf)

$(bin)/%.lst: $(bin)/%.so FORCE
	$(call if_changed_mkdir_dep,objd_lst_so)

$(bin)/%_strip.so: $(bin)/%.so FORCE
	$(call if_changed_mkdir_dep,strip_so_so)

# These implicit rules are evaluated by make-obj so each binary can have
# their own object subdirectory if needed (helpful for building the same
# source with and without fdpic enabled.
define imp-rules
$(1)/%.o: $(src)/%.c FORCE
	$$(call if_changed_mkdir_dep,cc_o_c)

$(1)/%.o: $(src)/%.s FORCE
	$$(call if_changed_mkdir_dep,cc_o_s)

$(1)/%.o: $(src)/%.S FORCE
	$$(call if_changed_mkdir_dep,cc_o_S)
endef


# File system and table objects/generated header.
ULD_FST_OBJ = $(obj)/uld_fst.o
ULD_FST_DATA_OBJ = $(addsuffix .o,$(basename $(ULD_FST_OBJ))_data)
ULD_GEN_FST_H = $(includedir)/generated/uld_fst.h

ULD_FILE_LIST =

# Create an empty object file to embed files into.
$(ULD_FST_DATA_OBJ): $(GEN_ULD_FILES_SCR)
	$(call if_changed_mkdir_dep,cc_o_null)

# Embed files and generated the fs table header.
$(ULD_GEN_FST_H): $(ULD_FST_DATA_OBJ)
	$(call if_changed_mkdir_dep,gen_uld_files)

$(ULD_FST_OBJ): $(ULD_GEN_FST_H)


ULD_SRC = \
	base64.c \
	cpu.c \
	elf.c \
	libc.c \
	swi.c \
	util.c \
	uld.c \
	uld_data.S \
	uld_dyn.c \
	uld_exec.c \
	uld_exec_asm.S \
	uld_file.c \
	uld_fs.c \
	uld_fst.S \
	uld_init.c \
	uld_load.c \
	uld_print.c \
	uld_reloc.c \
	uld_rofixup.c \
	uld_sal.c \
	uld_start.S \
	uld_vectors.S
$(call make-obj,$(bin)/uld.elf,$(ULD_SRC),ULD_OBJ)

ULD_BREAK_DEFS += -DULD_BREAK_BEFORE_CTOR
ULD_BREAK_DEFS += -DULD_BREAK_BEFORE_ENTRY
#ULD_BREAK_DEFS += -DULD_BREAK_BEFORE_STACK_RESET
$(call target_cflags,$(ULD_OBJ),$(NO_FDPIC) -D__ULD__ $(ULD_BREAK_DEFS))

$(call target_ldflags,$(bin)/uld.elf,$(NO_FDPIC))
$(bin)/uld.elf: LDSCRIPT_SUBTYPE =
$(bin)/uld.elf: $(ULD_OBJ) $(ULD_FST_DATA_OBJ) $(PATCH_ULD_ELF_SCR)
	$(call if_changed_mkdir_dep,link_elf_o_filt)

ULD_PATCHED_TARGET = $(bin)/.uld.elf.patched
$(ULD_PATCHED_TARGET): $(bin)/uld.elf
	$(call if_changed_mkdir_dep,patch_uld_elf)

-include $(call depfile-list,$(bin)/uld.elf $(bin)/uld.bin \
	$(bin)/uld.lst $(bin)/uld_strip.elf $(bin)/uld_gdb.elf)
$(bin)/uld.bin: $(ULD_PATCHED_TARGET)
$(bin)/uld.lst: $(ULD_PATCHED_TARGET)
$(bin)/uld_strip.elf: $(ULD_PATCHED_TARGET)
$(bin)/uld_gdb.elf: $(ULD_PATCHED_TARGET)

PHONY += uld
uld: $(bin)/uld.bin $(bin)/uld.lst $(bin)/uld_strip.elf $(bin)/uld_gdb.elf


include $(src)/example/Makefile


$(ULD_FST_DATA_OBJ): $(ULD_FILE_LIST)
$(ULD_GEN_FST_H): $(ULD_FILE_LIST)

PHONY += example_fw $(TARGETS)
example_fw: uld $(TARGETS)

PHONY += all
all: example_fw

PHONY += clean
clean: FORCE
	@for dir in $(sort $(CLEAN_DIRS)); do \
		echo $(RM) -rf $$dir; \
		$(RM) -rf $$dir; \
	done


PHONY += FORCE
FORCE:
.PHONY: $(PHONY)
