# FireRed PC — build (run from an MSYS2 MINGW32 shell, or via build.sh).
#
#   make            build FireRedPC.exe
#   make assets     (re)build pret's assets + reference ROM (needs devkitARM)
#
# Game C:   cpp -> pret preproc (INCBIN, charmap strings) -> i686 gcc
# Game data: pret's ARM objects -> tools/elf2x86.py -> i686 as

GAME   := game
BUILD  := build/pc
ARMOBJ := $(GAME)/build/firered_modern
EXE    := FireRedPC.exe

CC     := gcc
CXX    := g++
AS     := as
PY     := /c/Users/hyper/AppData/Local/Programs/Python/Python310/python
PREPROC := $(GAME)/tools/preproc/preproc.exe

GAME_DEFS := -D PLATFORM_PC -D MODERN=1 -D FIRERED=1 -D REVISION=0 -D ENGLISH=1 -D NDEBUG
# (paths relative to $(GAME): preproc resolves INCBIN paths from there)
GAME_CPPFLAGS := -iquote include -I ../port/include -Wno-trigraphs $(GAME_DEFS)
# Struct layouts must match the ARM build the data was assembled for.
GAME_CFLAGS := -pipe -m32 -O2 -g -std=gnu17 -fshort-enums -mno-ms-bitfields -fno-strict-aliasing -fwrapv \
               -fno-builtin-memcpy -w
PORT_CFLAGS := -pipe -m32 -O2 -g -std=gnu17 -Wall -Wno-unused-function -I port/include -iquote $(GAME)/include \
               $(GAME_DEFS) $(shell sdl2-config --cflags)
LDFLAGS := -m32 -g
LIBS := $(shell sdl2-config --libs) -lopengl32 -lwinmm -Wl,--defsym,_gNumMusicPlayers=4,--defsym,_gMaxLines=0

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

$(EXE): $(GAME_OBJS) $(DATA_OBJS) $(PORT_OBJS)
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
.SECONDARY:
