// Custom item entries — kept in separate file to avoid layout shifts in items.h
// Included at the end of gItems[] in items.h

    {
        .name = _("SHINY CHARM"),
        .itemId = ITEM_SHINY_CHARM,
        .price = 0,
        .holdEffect = HOLD_EFFECT_NONE,
        .holdEffectParam = 0,
        .description = gItemDescription_ITEM_SHINY_CHARM,
        .importance = 1,
        .registrability = 0,
        .pocket = POCKET_KEY_ITEMS,
        .type = ITEM_TYPE_BAG_MENU,
        .fieldUseFunc = FieldUseFunc_OakStopsYou,
        .battleUsage = 0,
        .battleUseFunc = NULL,
        .secondaryId = 0
    },
    {
        .name = _("POKé RADAR"),
        .itemId = ITEM_POKE_RADAR,
        .price = 0,
        .holdEffect = HOLD_EFFECT_NONE,
        .holdEffectParam = 0,
        .description = gItemDescription_ITEM_POKE_RADAR,
        .importance = 1,
        .registrability = 1,
        .pocket = POCKET_KEY_ITEMS,
        .type = ITEM_TYPE_FIELD,
        .fieldUseFunc = FieldUseFunc_PokeRadar,
        .battleUsage = 0,
        .battleUseFunc = NULL,
        .secondaryId = 0
    },
