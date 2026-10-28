# Runlink modules from C (included by the Makefile)
#
# A module is built from src/modules/mNNN.c when that file exists, and from
# asm/modules/mNNN.s (tools/runlink_split.py) otherwise. Module C is game
# code and uses the game flags (IDO 5.3 -O2 -mips1); INCLUDE_ASM pulls in the
# functions that are not decompiled yet from asm/nonmatchings/modules/mNNN/,
# and the module's data from data.s there. The object is relocatable: it is
# not linked; tools/runlink_build.py turns its sections and relocations into
# the module image and its runlink relocation tables. See docs/runlink.md.

MOD_C_DIR := src/modules
MOD_NAMES := $(basename $(notdir $(MOD_S)))
MOD_O     := $(foreach m,$(MOD_NAMES),$(if $(wildcard $(MOD_C_DIR)/$(m).c),$(BUILD_DIR)/$(MOD_C_DIR)/$(m).o,$(BUILD_DIR)/asm/modules/$(m).o))

MOD_CFLAGS := $(CFLAGS) -I include/modules

$(BUILD_DIR)/$(MOD_C_DIR)/%.o: $(MOD_C_DIR)/%.c
	@mkdir -p $(dir $@)
	$(PYTHON) $(ASM_PROC) --asm-prelude include/prelude.inc $(CC) -- $(AS) $(ASFLAGS) --no-warn -- \
	    -c $(MOD_CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<

# (the recipe in the Makefile uses the MOD_O above; it also reads
# undefined_syms for names given to main-segment addresses)
$(RL_DIR)/modules.bin: $(MOD_O) undefined_syms.$(VERSION).txt names.txt

# runlink_build.py writes the table together with the images; an (empty)
# recipe makes make look at the table again, so that the ROM is relinked in
# the same run when only the table changed
$(RL_DIR)/modtab.bin: $(RL_DIR)/modules.bin
	@true

# `make nonmatching` compiles every module C file with -DNON_MATCHING too (with
# the module flags, into build_nm/ like the main files): nothing else compiles
# the NON_MATCHING bodies, so a field rename or a header change that breaks one
# shows up here (and in CI) rather than when someone next works on it.
NM_MOD_O := $(patsubst %.c,$(NM_DIR)/%.o,$(filter $(MOD_C_DIR)/%,$(C_FILES)))

nonmatching: $(NM_MOD_O)

$(NM_DIR)/$(MOD_C_DIR)/%.o: $(MOD_C_DIR)/%.c
	@mkdir -p $(dir $@)
	$(PYTHON) $(ASM_PROC) --asm-prelude include/prelude.inc $(CC) -- $(AS) $(ASFLAGS) --no-warn -- \
	    -c $(MOD_CFLAGS) -DNON_MATCHING $(OPT_FLAGS) $(MIPSISET) -o $@ $<

.PHONY: modules-progress
modules-progress: $(TARGET).elf
	@$(PYTHON) tools/progress.py --modules
