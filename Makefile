# Jet Force Gemini matching decompilation
#
#   make setup     create the Python venv and install splat (once)
#   make extract   check the ROM, recover runlink symbols, split with splat
#   make           assemble and link build/jfg.us.z64 and check its SHA-1

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

CROSS   ?= mips-linux-gnu-
AS      := $(CROSS)as
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

ASFLAGS := -EB -march=vr4300 -mtune=vr4300 -mabi=32 -G 0 -I include -I .
LDFLAGS := -T $(LD_SCRIPT) \
           -T $(SYMS_DIR)/undefined_syms_auto.$(VERSION).txt \
           -T $(SYMS_DIR)/undefined_funcs_auto.$(VERSION).txt \
           -T undefined_syms.$(VERSION).txt \
           -Map $(TARGET).map --no-check-sections

S_FILES   := $(shell find asm -name '*.s' 2>/dev/null)
BIN_FILES := $(shell find assets -name '*.bin' 2>/dev/null)
O_FILES   := $(S_FILES:%.s=$(BUILD_DIR)/%.o) $(BIN_FILES:assets/%.bin=$(BUILD_DIR)/assets/%.o)

.PHONY: all setup extract check clean distclean verify
.SECONDARY:
SHELL := /bin/bash -e -o pipefail

all: verify

setup:
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install -r requirements.txt

check: .baserom.$(VERSION).ok

.baserom.$(VERSION).ok: $(BASEROM)
	@sha1sum --check $(SHA1FILE)
	@touch $@

extract: check
	@mkdir -p $(SYMS_DIR)
	$(PYTHON) tools/runlink_syms.py $(BASEROM) $(SYMS_DIR)/runlink_syms.$(VERSION).txt \
	    --version $(VERSION) --modules $(SYMS_DIR)/modules.$(VERSION).txt
	$(PYTHON) -m splat split $(YAML)
	@# spimdisasm runs some jump tables on into the zero padding after them
	@# and emits those words as a label at address 0; keep them as data.
	sed -i 's/\.word \.L0$$/.word 0x00000000/' asm/data/main.rodata.s

verify: $(TARGET).z64
	@echo "$$(cut -d' ' -f1 $(SHA1FILE))  $(TARGET).z64" | sha1sum --check

$(TARGET).elf: $(O_FILES) $(LD_SCRIPT)
	$(LD) $(LDFLAGS) -o $@

$(TARGET).z64: $(TARGET).elf
	$(OBJCOPY) -O binary --pad-to=0x2000000 --gap-fill=0x00 $< $@

$(BUILD_DIR)/asm/%.o: asm/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/assets/%.o: assets/%.bin
	@mkdir -p $(dir $@)
	$(LD) -r -b binary -o $@ $<

clean:
	rm -rf $(BUILD_DIR)/asm $(BUILD_DIR)/assets $(TARGET).*

# also removes everything extract generated from the ROM
distclean:
	rm -rf $(BUILD_DIR) asm assets $(LD_SCRIPT) .baserom.$(VERSION).ok
