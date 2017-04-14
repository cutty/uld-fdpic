###############################################################################
# hello_world_static.elf
###############################################################################
APP_HELLO_WORLD_STATIC_SRC = \
	example/ex_app_hello_world.c \
	exec_vectors.S \
	libc.c \
	swi.c \
	util.c
$(call make-obj, \
	$(bin)/hello_world_static.elf, \
	$(APP_HELLO_WORLD_STATIC_SRC), \
	APP_HELLO_WORLD_STATIC_OBJ, \
	hello_world_static)
$(bin)/hello_world_static.elf: LIBS =
$(bin)/hello_world_static.elf: LDSCRIPT_SUBTYPE = app
$(bin)/hello_world_static.elf: $(APP_HELLO_WORLD_STATIC_OBJ)
	$(call if_changed_mkdir_dep,link_elf_o_filt)

-include $(call depfile-list, \
	$(bin)/hello_world_static.elf \
	$(bin)/hello_world_static.lst \
	$(bin)/hello_world_static_strip.elf)
$(bin)/hello_world_static.lst: $(bin)/hello_world_static.elf
$(bin)/hello_world_static_strip.elf: $(bin)/hello_world_static.elf

hello_world_static: \
	$(bin)/hello_world_static.lst \
	$(bin)/hello_world_static_strip.elf

TARGETS += hello_world_static

ULD_FILE_LIST += \
	$(bin)/hello_world_static_strip.elf


###############################################################################
# libexc.so
###############################################################################
LIBEXC_SRC = \
	libc.c \
	swi.c \
	util.c
$(call make-obj, \
	$(bin)/libexc.so, \
	$(LIBEXC_SRC), \
	LIBEXC_OBJ, \
	libexc)
$(call target_cflags,$(LIBEXC_OBJ),$(CFLAGS_SO))
$(bin)/libexc.so: LIBS =
$(bin)/libexc.so: LDFLAGS_EXTRA = -Wl,-soname,libexc.so
$(bin)/libexc.so: LDSCRIPT_SUBTYPE = so
$(bin)/libexc.so: $(LIBEXC_OBJ)
	$(call if_changed_mkdir_dep,link_so_o)

-include $(call depfile-list, \
	$(bin)/libexc.so \
	$(bin)/libexc.lst \
	$(bin)/libexc_strip.so)
$(bin)/libexc.lst: $(bin)/libexc.so
$(bin)/libexc_strip.elf: $(bin)/libexc.so

libexc: \
	$(bin)/libexc.lst \
	$(bin)/libexc_strip.so

TARGETS += libexc

ULD_FILE_LIST += \
	$(bin)/libexc_strip.so


###############################################################################
# hello_world.elf
###############################################################################
APP_HELLO_WORLD_SRC = \
	example/ex_app_hello_world.c \
	exec_vectors.S
$(call make-obj, \
	$(bin)/hello_world.elf, \
	$(APP_HELLO_WORLD_SRC), \
	APP_HELLO_WORLD_OBJ, \
	hello_world)
$(call target_cflags,$(APP_HELLO_WORLD_OBJ),-DHELLO_WORLD_DYNAMIC)
$(bin)/hello_world.elf: LIBS = exc
$(bin)/hello_world.elf: LDSCRIPT_SUBTYPE = app
$(bin)/hello_world.elf: $(bin)/libexc.so
$(bin)/hello_world.elf: $(APP_HELLO_WORLD_OBJ)
	$(call if_changed_mkdir_dep,link_elf_o_filt)

-include $(call depfile-list, \
	$(bin)/hello_world.elf \
	$(bin)/hello_world.lst \
	$(bin)/hello_world_strip.elf)
$(bin)/hello_world.lst: $(bin)/hello_world.elf
$(bin)/hello_world_strip.elf: $(bin)/hello_world.elf

hello_world: \
	$(bin)/hello_world.lst \
	$(bin)/hello_world_strip.elf

TARGETS += hello_world

ULD_FILE_LIST += \
	$(bin)/hello_world_strip.elf


###############################################################################
# libftable.so
###############################################################################
LIBFTABLE_SRC = \
	example/ex_lib_ftable.c
$(call make-obj, \
	$(bin)/libftable.so, \
	$(LIBFTABLE_SRC), \
	LIBFTABLE_OBJ, \
	libftable)
$(call target_cflags,$(LIBFTABLE_OBJ),$(CFLAGS_SO))
$(bin)/libftable.so: LIBS = exc
$(bin)/libftable.so: LDFLAGS_EXTRA = -Wl,-soname,libftable.so
$(bin)/libftable.so: LDSCRIPT_SUBTYPE = so
$(bin)/libftable.so: $(bin)/libexc.so
$(bin)/libftable.so: $(LIBFTABLE_OBJ)
	$(call if_changed_mkdir_dep,link_so_o)

-include $(call depfile-list, \
	$(bin)/libftable.so \
	$(bin)/libftable.lst \
	$(bin)/libftable_strip.so)
$(bin)/libftable.lst: $(bin)/libftable.so
$(bin)/libftable_strip.elf: $(bin)/libftable.so

libftable: \
	$(bin)/libftable.lst \
	$(bin)/libftable_strip.so

TARGETS += libftable

ULD_FILE_LIST += \
	$(bin)/libftable_strip.so


###############################################################################
# libprint.so
###############################################################################
LIBPRINT_SRC = \
	example/ex_lib_print.c
$(call make-obj, \
	$(bin)/libprint.so, \
	$(LIBPRINT_SRC), \
	LIBPRINT_OBJ, \
	libprint)
$(call target_cflags,$(LIBPRINT_OBJ),$(CFLAGS_SO))
$(bin)/libprint.so: LIBS = exc
$(bin)/libprint.so: LDFLAGS_EXTRA = -Wl,-soname,libprint.so
$(bin)/libprint.so: LDSCRIPT_SUBTYPE = so
$(bin)/libprint.so: $(bin)/libexc.so
$(bin)/libprint.so: $(LIBPRINT_OBJ)
	$(call if_changed_mkdir_dep,link_so_o)

-include $(call depfile-list, \
	$(bin)/libprint.so \
	$(bin)/libprint.lst \
	$(bin)/libprint_strip.so)
$(bin)/libprint.lst: $(bin)/libprint.so
$(bin)/libprint_strip.elf: $(bin)/libprint.so

libprint: \
	$(bin)/libprint.lst \
	$(bin)/libprint_strip.so

TARGETS += libprint

ULD_FILE_LIST += \
	$(bin)/libprint_strip.so


###############################################################################
# libgarage.so
###############################################################################
LIBGARAGE_SRC = \
	example/ex_lib_garage.c
$(call make-obj, \
	$(bin)/libgarage.so, \
	$(LIBGARAGE_SRC), \
	LIBGARAGE_OBJ, \
	libgarage)
$(call target_cflags,$(LIBGARAGE_OBJ),$(CFLAGS_SO))
$(bin)/libgarage.so: LIBS = print exc
$(bin)/libgarage.so: LDFLAGS_EXTRA = -Wl,-soname,libgarage.so
$(bin)/libgarage.so: LDSCRIPT_SUBTYPE = so
$(bin)/libgarage.so: $(bin)/libexc.so
$(bin)/libgarage.so: $(bin)/libprint.so
$(bin)/libgarage.so: $(LIBGARAGE_OBJ)
	$(call if_changed_mkdir_dep,link_so_o)

-include $(call depfile-list, \
	$(bin)/libgarage.so \
	$(bin)/libgarage.lst \
	$(bin)/libgarage_strip.so)
$(bin)/libgarage.lst: $(bin)/libgarage.so
$(bin)/libgarage_strip.elf: $(bin)/libgarage.so

libgarage: \
	$(bin)/libgarage.lst \
	$(bin)/libgarage_strip.so

TARGETS += libgarage

ULD_FILE_LIST += \
	$(bin)/libgarage_strip.so


###############################################################################
# dyn_test.elf
###############################################################################
APP_DYN_TEST_SRC = \
	example/ex_app_dyn_test.c \
	exec_vectors.S
$(call make-obj, \
	$(bin)/dyn_test.elf, \
	$(APP_DYN_TEST_SRC), \
	APP_DYN_TEST_OBJ, \
	dyn_test)
$(bin)/dyn_test.elf: LIBS = ftable garage exc
$(bin)/dyn_test.elf: LDSCRIPT_SUBTYPE = app
$(bin)/dyn_test.elf: $(bin)/libexc.so
$(bin)/dyn_test.elf: $(bin)/libgarage.so
$(bin)/dyn_test.elf: $(bin)/libftable.so
$(bin)/dyn_test.elf: $(APP_DYN_TEST_OBJ)
	$(call if_changed_mkdir_dep,link_elf_o_filt)

-include $(call depfile-list, \
	$(bin)/dyn_test.elf \
	$(bin)/dyn_test.lst \
	$(bin)/dyn_test_strip.elf)
$(bin)/dyn_test.lst: $(bin)/dyn_test.elf
$(bin)/dyn_test_strip.elf: $(bin)/dyn_test.elf

dyn_test: \
	$(bin)/dyn_test.lst \
	$(bin)/dyn_test_strip.elf

TARGETS += dyn_test

ULD_FILE_LIST += \
	$(bin)/dyn_test_strip.elf
