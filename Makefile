# Jet Force Gemini Decomped: matching decompilation build
#
#   make setup     Python venv (splat), IDO 5.3 recomp, m2c, submodules (once)
#   make extract   check the ROM, recover runlink symbols, split with splat
#   make           compile, assemble and link build/jfg.us.z64, check its SHA-1
#   make check     check the SHA-1 of the built ROM
#   make progress  functions decompiled to C, per segment

VERSION  ?= us
BASENAME := jfg
BASEROM  := baserom.$(VERSION).z64
YAML     := $(BASENAME).$(VERSION).yaml
LD_SCRIPT:= $(BASENAME).$(VERSION).ld
SHA1FILE := $(BASENAME).$(VERSION).sha1

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/$(BASENAME).$(VERSION)
SYMS_DIR  := $(BUILD_DIR)/syms

VENV   := .venv
PYTHON := $(VENV)/bin/python3

# ---- tools -----------------------------------------------------------------

CROSS   ?= mips-linux-gnu-
AS      := $(CROSS)as
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

# IDO 5.3, statically recompiled (decompals/ido-static-recomp release, fetched
# by `make setup`, not part of this repository)
IDO_VERSION := v1.2
IDO_DIR     := tools/ido5.3
CC          := $(IDO_DIR)/cc
# m2c (GPL-3.0) is fetched by `make setup` and only run as a tool
M2C_DIR     := tools/m2c
M2C_COMMIT  := c632946b8cfbe8fce442b522dba92dfa72f1f317
ASM_PROC    := tools/asm-processor/build.py
UL_DIR      := lib/ultralib

# ---- flags -----------------------------------------------------------------

ASFLAGS := -EB -march=vr4300 -mtune=vr4300 -mabi=32 -G 0 -I include -I .

# Game code: IDO 5.3 -O2 -mips1 (load-delay nops, no branch-likely)
INCLUDES := -I include -I src -I $(UL_DIR)/include -I $(UL_DIR)/include/PR \
            -I $(UL_DIR)/include/compiler/ido
DEFINES  := -D_LANGUAGE_C -D_MIPS_SZLONG=32 -D_FINALROM -DNDEBUG \
            -DBUILD_VERSION=VERSION_I
CFLAGS   := -G 0 -non_shared -Xcpluscomm -nostdinc -Wab,-r4300_mul \
            -woff 516,608,649,838,712 $(DEFINES) $(INCLUDES)
OPT_FLAGS := -O2
MIPSISET  := -mips1

# libultra objects built from decompals/ultralib (src/ultra/*.c wrappers),
# with ultralib's flags for SDK 2.0I; per-object levels in src/ultra/flags.mk
UL_CFLAGS := -G 0 -non_shared -Xcpluscomm -nostdinc -Wab,-r4300_mul \
             -woff 516,649,838,712 -D_MIPS_SZLONG=32 -D_FINALROM -DNDEBUG \
             -DBUILD_VERSION=VERSION_I -DBUILD_VERSION_STRING=\"2.0I\" -DF3DEX_GBI_2 \
             -I $(UL_DIR) -I $(UL_DIR)/include -I $(UL_DIR)/include/compiler/ido \
             -I $(UL_DIR)/include/PR
# Nintendo's optimisation levels for SDK 2.0I: os and io at -O1, the rest at
# -O3; the long long helpers are -mips3 code.
UL_OPT_FLAGS = $(if $(filter $(UL_O2_OBJS),$(patsubst $(BUILD_DIR)/src/ultra/%.o,%,$@)),-O2,$(if $(filter os io,$(word 4,$(subst /, ,$@)))$(filter ll llcvt,$(basename $(notdir $@))),-O1,-O3))
# A few objects are the later (2.0J) form, built at -O2 like 2.0J's libultra
UL_O2_OBJS   := io/pirawdma io/pigetcmdq
UL_MIPSISET  = $(if $(filter ll llcvt,$(basename $(notdir $@))),-mips3 -32,-mips2)

LDFLAGS := -T $(LD_SCRIPT) \
           -T $(SYMS_DIR)/undefined_syms_auto.$(VERSION).txt \
           -T $(SYMS_DIR)/undefined_funcs_auto.$(VERSION).txt \
           -T $(SYMS_DIR)/runlink_aliases.$(VERSION).txt \
           -T undefined_syms.$(VERSION).txt \
           -Map $(TARGET).map --no-check-sections --no-warn-mismatch

# ---- sources ---------------------------------------------------------------

# asm/nonmatchings/ holds one file per function, pulled into C by INCLUDE_ASM
S_FILES   := $(shell find asm -name '*.s' -not -path 'asm/nonmatchings/*' 2>/dev/null)
C_FILES   := $(shell find src -name '*.c' 2>/dev/null)
BIN_FILES := $(shell find assets -name '*.bin' 2>/dev/null)
O_FILES   := $(S_FILES:%.s=$(BUILD_DIR)/%.o) \
             $(C_FILES:%.c=$(BUILD_DIR)/%.o) \
             $(BIN_FILES:assets/%.bin=$(BUILD_DIR)/assets/%.o)

.PHONY: all setup extract check verify progress clean distclean
.SECONDARY:
SHELL := /bin/bash -e -o pipefail

all: check

setup:
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install -r requirements.txt
	git submodule update --init
	mkdir -p $(IDO_DIR)
	curl -sSfL https://github.com/decompals/ido-static-recomp/releases/download/$(IDO_VERSION)/ido-5.3-recomp-linux.tar.gz \
	    | tar xz -C $(IDO_DIR)
	if [ ! -d $(M2C_DIR) ]; then git clone -q https://github.com/matt-kempster/m2c.git $(M2C_DIR); fi
	git -C $(M2C_DIR) checkout -q $(M2C_COMMIT)

.baserom.$(VERSION).ok: $(BASEROM)
	@sha1sum --check $(SHA1FILE)
	@touch $@

extract: .baserom.$(VERSION).ok
	rm -rf asm assets
	@mkdir -p $(SYMS_DIR)
	$(PYTHON) tools/runlink_syms.py $(BASEROM) $(SYMS_DIR)/runlink_syms.$(VERSION).txt \
	    --version $(VERSION) --modules $(SYMS_DIR)/modules.$(VERSION).txt \
	    --aliases $(SYMS_DIR)/runlink_aliases.$(VERSION).txt
	$(PYTHON) -m splat split $(YAML)
	@# spimdisasm runs some jump tables on into the zero padding after them
	@# and emits those words as a label at address 0; keep them as data.
	sed -i 's/\.word \.L0$$/.word 0x00000000/' asm/data/main.rodata.s
	@# INCLUDE_ASM'd objects must be rebuilt from the fresh asm
	rm -rf $(BUILD_DIR)/src

check verify: $(TARGET).z64
	@echo "$$(cut -d' ' -f1 $(SHA1FILE))  $(TARGET).z64" | sha1sum --check

progress: $(TARGET).elf
	@$(PYTHON) tools/progress.py

$(TARGET).elf: $(O_FILES) $(LD_SCRIPT)
	$(LD) $(LDFLAGS) -o $@

$(TARGET).z64: $(TARGET).elf
	$(OBJCOPY) -O binary --pad-to=0x2000000 --gap-fill=0x00 $< $@

$(BUILD_DIR)/asm/%.o: asm/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) --no-warn -o $@ $<

# libultra: src/ultra/<dir>/<file>.c just #includes ultralib's source
$(BUILD_DIR)/src/ultra/%.o: src/ultra/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $(UL_CFLAGS) $(UL_OPT_FLAGS) $(UL_MIPSISET) -o $@ $<

$(BUILD_DIR)/src/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(PYTHON) $(ASM_PROC) --asm-prelude include/prelude.inc $(CC) -- $(AS) $(ASFLAGS) --no-warn -- \
	    -c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<

$(BUILD_DIR)/assets/%.o: assets/%.bin
	@mkdir -p $(dir $@)
	$(LD) -r -b binary -o $@ $<

-include $(shell find $(BUILD_DIR)/src -name '*.d' 2>/dev/null)

clean:
	rm -rf $(BUILD_DIR)/asm $(BUILD_DIR)/src $(BUILD_DIR)/assets $(TARGET).*

# also removes everything extract generated from the ROM
distclean:
	rm -rf $(BUILD_DIR) asm assets $(LD_SCRIPT) .baserom.$(VERSION).ok
