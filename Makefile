# FireRed PC — build (run from an MSYS2 MINGW32 shell, or via build.sh).
#
#   make            build FireRedPC.exe
#   make VERSION=LEAFGREEN   build LeafGreenPC.exe (needs `make leafgreen_modern` in game/ first)
#   make assets     (re)build pret's assets + reference ROM (needs devkitARM)
#
# Game C:   cpp -> pret preproc (INCBIN, charmap strings) -> i686 gcc
# Game data: pret's ARM objects -> tools/elf2x86.py -> i686 as

VERSION ?= FIRERED
GAME   := game
ifeq ($(VERSION),LEAFGREEN)
BUILD  := build/pc_lg
ARMOBJ := $(GAME)/build/leafgreen_modern
EXE    := LeafGreenPC.exe
VERSION_DEF := -D LEAFGREEN=1
SHA1S  := game/leafgreen.sha1 game/leafgreen_rev1.sha1
else
BUILD  := build/pc
ARMOBJ := $(GAME)/build/firered_modern
EXE    := FireRedPC.exe
VERSION_DEF := -D FIRERED=1
SHA1S  := game/firered.sha1 game/firered_rev1.sha1
endif

CC     := gcc
CXX    := g++
AS     := as
PY     := /c/Users/hyper/AppData/Local/Programs/Python/Python310/python
PREPROC := $(GAME)/tools/preproc/preproc.exe

GAME_DEFS := -D PLATFORM_PC -D MODERN=1 $(VERSION_DEF) -D REVISION=0 -D ENGLISH=1 -D NDEBUG
# (paths relative to $(GAME): preproc resolves INCBIN paths from there)
GAME_CPPFLAGS := -iquote include -I ../port/include -Wno-trigraphs $(GAME_DEFS)
# Struct layouts must match the ARM build the data was assembled for.
GAME_CFLAGS := -pipe -m32 -O2 -g -std=gnu17 -fshort-enums -mno-ms-bitfields -fno-strict-aliasing -fwrapv \
               -fno-builtin-memcpy -fno-toplevel-reorder -w
PORT_CFLAGS := -pipe -m32 -O2 -g -std=gnu17 -Wall -Wno-unused-function -I port/include -iquote $(GAME)/include \
               $(GAME_DEFS) $(shell sdl2-config --cflags)
LDFLAGS := -m32 -g
LIBS := $(shell sdl2-config --libs) -lopengl32 -lwinmm -lws2_32 -lcomdlg32 -Wl,--defsym,_gNumMusicPlayers=4,--defsym,_gMaxLines=0

# Game sources that are replaced or have no PC equivalent.
GAME_SKIP := isagbprn.c mini_printf.c librfu_intr.c multiboot.c agb_flash.c agb_flash_1m.c agb_flash_le.c agb_flash_mx.c
GAME_SRCS := $(filter-out $(addprefix $(GAME)/src/,$(GAME_SKIP)),$(wildcard $(GAME)/src/*.c))
GAME_OBJS := $(patsubst $(GAME)/src/%.c,$(BUILD)/game/%.o,$(GAME_SRCS))

DATA_ARM  := $(filter-out $(ARMOBJ)/data/mystery_event_msg.o,$(wildcard $(ARMOBJ)/data/*.o)) $(wildcard $(ARMOBJ)/sound/songs/midi/*.o)
DATA_OBJS := $(patsubst $(ARMOBJ)/%.o,$(BUILD)/data/%.o,$(DATA_ARM))

PORT_SRCS := $(wildcard port/src/*.c)
PORT_OBJS := $(patsubst port/src/%.c,$(BUILD)/port/%.o,$(PORT_SRCS))

# Native gcc/collect2 need a Windows temp dir.
$(shell mkdir -p build/tmp)
export TMP := $(shell cygpath -m $(CURDIR)/build/tmp)
export TEMP := $(TMP)

all: $(EXE)

# The ROM's content is taken out of the exe (tools/rompack.c) when a retail ROM
# is found in ROM_DIRS; players' copies unpack it from their own cartridge.
ROM_DIRS ?= .
ROMPACK := build/rompack.exe

$(EXE): $(BUILD)/game.exe $(ROMPACK) tools/game_ranges.py
	$(PY) tools/game_ranges.py $(BUILD)/FireRedPC.map $(BUILD)/game_ranges.txt $(BUILD)/game/ $(BUILD)/data/
	$(ROMPACK) $(BUILD)/game.exe $@ --allow $(BUILD)/game_ranges.txt $(foreach f,$(SHA1S),--sha1 $(firstword $(shell cat $(f)))) $(foreach d,$(ROM_DIRS),--dir $(d))

$(ROMPACK): tools/rompack.c
	/c/msys64/mingw64/bin/gcc -O2 -o $@ $<

$(BUILD)/game.exe: $(GAME_OBJS) $(DATA_OBJS) $(PORT_OBJS)
	@echo "LD $@"
	@printf '%s\n' $^ > $(BUILD)/objs.rsp
	@$(CC) $(LDFLAGS) -o $@ @$(BUILD)/objs.rsp $(LIBS) -Wl,-Map=$(BUILD)/FireRedPC.map

# preproc resolves INCBIN paths relative to the game directory.
$(BUILD)/game/%.o: $(GAME)/src/%.c
	@mkdir -p $(dir $@)
	cd $(GAME) && $(CC) -E -m32 $(GAME_CPPFLAGS) -MMD -MF ../$@.d -MT ../$@ src/$*.c \
	  | tools/preproc/preproc.exe -i src/$*.c charmap.txt \
	  | $(CC) $(GAME_CFLAGS) -x c -c - -o ../$@

$(BUILD)/data/%.s: $(ARMOBJ)/%.o tools/elf2x86.py
	@mkdir -p $(dir $@)
	$(PY) tools/elf2x86.py $< $@

$(BUILD)/data/%.o: $(BUILD)/data/%.s
	$(AS) --32 -o $@ $<

$(BUILD)/port/%.o: port/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) -MMD -c $< -o $@

assets:
	cd $(GAME) && PATH=/c/devkitPro/devkitARM/bin:$$PATH $(MAKE) modern -j8

clean:
	rm -rf $(BUILD) $(EXE)

-include $(GAME_OBJS:.o=.o.d) $(PORT_OBJS:.o=.d)
.PHONY: all assets clean
.PRECIOUS: $(BUILD)/data/%.s
