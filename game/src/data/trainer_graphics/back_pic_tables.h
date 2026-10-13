const struct MonCoords gTrainerBackPicCoords[] = {
    [TRAINER_BACK_PIC_RED] = {.size = 8, .y_offset = 5},
    [TRAINER_BACK_PIC_LEAF] = {.size = 8, .y_offset = 5},
    [TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_RUBY_SAPPHIRE_MAY] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_POKEDUDE] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_OLD_MAN] = {.size = 8, .y_offset = 4},
#ifdef PLATFORM_PC
    // a visitor from Hoenn: Emerald's Brendan and May (pc_visitor_gfx.h)
    [TRAINER_BACK_PIC_PC_VISITOR(MALE)] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_PC_VISITOR(FEMALE)] = {.size = 8, .y_offset = 4},
#endif
};

const struct CompressedSpriteSheet gTrainerBackPicTable[] = {
    { (const u32 *)gTrainerBackPic_Red, 0x2800, 0 },
    { (const u32 *)gTrainerBackPic_Leaf, 0x2800, 1 },
    { (const u32 *)gTrainerBackPic_RSBrendan, 0x2000, 2 },
    { (const u32 *)gTrainerBackPic_RSMay, 0x2000, 3 },
    { (const u32 *)gTrainerBackPic_Pokedude, 0x2000, 4 },
    { (const u32 *)gTrainerBackPic_OldMan, 0x2000, 5 },
#ifdef PLATFORM_PC
    { (const u32 *)gPcVisitorBackPic_M, 0x2000, 6 },
    { (const u32 *)gPcVisitorBackPic_F, 0x2000, 7 },
#endif
};

const struct CompressedSpritePalette gTrainerBackPicPaletteTable[] = {
    { gTrainerPalette_RedBackPic, 0 },
    { gTrainerPalette_LeafBackPic, 1 },
    { gTrainerPalette_RSBrendan1, 2 },
    { gTrainerPalette_RSMay1, 3 },
    { gTrainerPalette_PokedudeBackPic, 4 },
    { gTrainerPalette_OldManBackPic, 5 },
#ifdef PLATFORM_PC
    { gPcVisitorTrainerPalette_M, 6 },
    { gPcVisitorTrainerPalette_F, 7 },
#endif
};
