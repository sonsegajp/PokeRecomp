
	.macro nop
	.byte 0x00
	.endm


	.macro nop1
	.byte 0x01
	.endm


	.macro end
	.byte 0x02
	.endm


	.macro return
	.byte 0x03
	.endm


	.macro call destination:req
	.byte 0x04
	.4byte \destination
	.endm


	.macro goto destination:req
	.byte 0x05
	.4byte \destination
	.endm


	.macro goto_if condition:req, destination:req
	.byte 0x06
	.byte \condition
	.4byte \destination
	.endm


	.macro call_if condition:req, destination:req
	.byte 0x07
	.byte \condition
	.4byte \destination
	.endm


	.macro gotostd function:req
	.byte 0x08
	.byte \function
	.endm


	STD_OBTAIN_ITEM = 0
	STD_FIND_ITEM = 1

	MSGBOX_NPC = 2
	MSGBOX_SIGN = 3
	MSGBOX_DEFAULT = 4
	MSGBOX_YESNO = 5
	MSGBOX_AUTOCLOSE = 6

	STD_OBTAIN_DECORATION = 7
	STD_PUT_ITEM_AWAY = 8
	STD_RECEIVED_ITEM = 9


	.macro callstd function:req
	.byte 0x09
	.byte \function
	.endm


	.macro gotostd_if condition:req, function:req
	.byte 0x0a
	.byte \condition
	.byte \function
	.endm


	.macro callstd_if condition:req, function:req
	.byte 0x0b
	.byte \condition
	.byte \function
	.endm


	.macro returnram
	.byte 0x0c
	.endm


	.macro endram
	.byte 0x0d
	.endm


	.macro setmysteryeventstatus value:req
	.byte 0x0e
	.byte \value
	.endm


	.macro loadword destIndex:req, value:req
	.byte 0x0f
	.byte \destIndex
	.4byte \value
	.endm


	.macro loadbyte destIndex:req, value:req
	.byte 0x10
	.byte \destIndex
	.byte \value
	.endm


	.macro setptr value:req, ptr:req
	.byte 0x11
	.byte \value
	.4byte \ptr
	.endm


	.macro loadbytefromptr destIndex:req, source:req
	.byte 0x12
	.byte \destIndex
	.4byte \source
	.endm


	.macro setptrbyte srcIndex:req, destination:req
	.byte 0x13
	.byte \srcIndex
	.4byte \destination
	.endm


	.macro copylocal destIndex:req, srcIndex:req
	.byte 0x14
	.byte \destIndex
	.byte \srcIndex
	.endm


	.macro copybyte destination:req, source:req
	.byte 0x15
	.4byte \destination
	.4byte \source
	.endm


	.macro setvar destination:req, value:req
	.byte 0x16
	.2byte \destination
	.2byte \value
	.endm


	.macro addvar destination:req, value:req
	.byte 0x17
	.2byte \destination
	.2byte \value
	.endm


	.macro subvar destination:req, value:req
	.byte 0x18
	.2byte \destination
	.2byte \value
	.endm


	.macro copyvar destination:req, source:req
	.byte 0x19
	.2byte \destination
	.2byte \source
	.endm


	.macro setorcopyvar destination:req, source:req
	.byte 0x1a
	.2byte \destination
	.2byte \source
	.endm



	.macro compare_local_to_local local1:req, local2:req
	.byte 0x1b
	.byte \local1
	.byte \local2
	.endm



	.macro compare_local_to_value local:req, value:req
	.byte 0x1c
	.byte \local
	.byte \value
	.endm



	.macro compare_local_to_ptr local:req, ptr:req
	.byte 0x1d
	.byte \local
	.4byte \ptr
	.endm



	.macro compare_ptr_to_local ptr:req, local:req
	.byte 0x1e
	.4byte \ptr
	.byte \local
	.endm



	.macro compare_ptr_to_value ptr:req, value:req
	.byte 0x1f
	.4byte \ptr
	.byte \value
	.endm



	.macro compare_ptr_to_ptr ptr1:req, ptr2:req
	.byte 0x20
	.4byte \ptr1
	.4byte \ptr2
	.endm



	.macro compare_var_to_value var:req, value:req
	.byte 0x21
	.2byte \var
	.2byte \value
	.endm



	.macro compare_var_to_var var1:req, var2:req
	.byte 0x22
	.2byte \var1
	.2byte \var2
	.endm



	.macro compare var:req, arg:req
		.if ((\arg >= VARS_START && \arg <= VARS_END) || (\arg >= SPECIAL_VARS_START && \arg <= SPECIAL_VARS_END))
			compare_var_to_var \var, \arg
		.else
			compare_var_to_value \var, \arg
		.endif
	.endm


	.macro callnative func:req
	.byte 0x23
	.4byte \func
	.endm


	.macro gotonative func:req
	.byte 0x24
	.4byte \func
	.endm


	.macro special function:req
	.byte 0x25
	.2byte SPECIAL_\function
	.endm



	.macro specialvar output:req, function:req
	.byte 0x26
	.2byte \output
	.2byte SPECIAL_\function
	.endm



	.macro waitstate
	.byte 0x27
	.endm


	.macro delay frames:req
	.byte 0x28
	.2byte \frames
	.endm


	.macro setflag flag:req
	.byte 0x29
	.2byte \flag
	.endm


	.macro clearflag flag:req
	.byte 0x2a
	.2byte \flag
	.endm



	.macro checkflag flag:req
	.byte 0x2b
	.2byte \flag
	.endm


	.macro initclock hour:req, minute:req
	.byte 0x2c
	.2byte \hour
	.2byte \minute
	.endm


	.macro dotimebasedevents
	.byte 0x2d
	.endm


	.macro gettime
	.byte 0x2e
	.endm


	.macro playse song:req
	.byte 0x2f
	.2byte \song
	.endm


	.macro waitse
	.byte 0x30
	.endm


	.macro playfanfare song:req
	.byte 0x31
	.2byte \song
	.endm


	.macro waitfanfare
	.byte 0x32
	.endm



	.macro playbgm song:req, save_song:req
	.byte 0x33
	.2byte \song
	.byte \save_song
	.endm



	.macro savebgm song:req
	.byte 0x34
	.2byte \song
	.endm


	.macro fadedefaultbgm
	.byte 0x35
	.endm


	.macro fadenewbgm song:req
	.byte 0x36
	.2byte \song
	.endm


	.macro fadeoutbgm speed:req
	.byte 0x37
	.byte \speed
	.endm


	.macro fadeinbgm speed:req
	.byte 0x38
	.byte \speed
	.endm











	.macro formatwarp map:req, a, b, c
	map \map
	.ifb \a
		.byte WARP_ID_NONE
		.2byte -1
		.2byte -1
	.else
	.ifb \b
		.byte \a  
		.2byte -1 
		.2byte -1 
	.else
	.ifb \c
		.byte WARP_ID_NONE
		.2byte \a 
		.2byte \b 
	.else
		.byte \a 
		.2byte \b
		.2byte \c
	.endif
	.endif
	.endif
	.endm




	.macro warp map:req, a, b, c
	.byte 0x39
	formatwarp \map, \a, \b, \c
	.endm




	.macro warpsilent map:req, a, b, c
	.byte 0x3a
	formatwarp \map, \a, \b, \c
	.endm




	.macro warpdoor map:req, a, b, c
	.byte 0x3b
	formatwarp \map, \a, \b, \c
	.endm




	.macro warphole map:req
	.byte 0x3c
	map \map
	.endm





	.macro warpteleport map:req, a, b, c
	.byte 0x3d
	formatwarp \map, \a, \b, \c
	.endm




	.macro setwarp map:req, a, b, c
	.byte 0x3e
	formatwarp \map, \a, \b, \c
	.endm




	.macro setdynamicwarp map:req, a, b, c
	.byte 0x3f
	formatwarp \map, \a, \b, \c
	.endm






	.macro setdivewarp map:req, a, b, c
	.byte 0x40
	formatwarp \map, \a, \b, \c
	.endm







	.macro setholewarp map:req, a=0, b=0, c
	.byte 0x41
	formatwarp \map, \a, \b, \c
	.endm


	.macro getplayerxy x:req, y:req
	.byte 0x42
	.2byte \x
	.2byte \y
	.endm


	.macro getpartysize
	.byte 0x43
	.endm



	.macro additem itemId:req, quantity=1
	.byte 0x44
	.2byte \itemId
	.2byte \quantity
	.endm



	.macro removeitem itemId:req, quantity=1
	.byte 0x45
	.2byte \itemId
	.2byte \quantity
	.endm



	.macro checkitemspace itemId:req, quantity=1
	.byte 0x46
	.2byte \itemId
	.2byte \quantity
	.endm



	.macro checkitem itemId:req, quantity=1
	.byte 0x47
	.2byte \itemId
	.2byte \quantity
	.endm



	.macro checkitemtype itemId:req
	.byte 0x48
	.2byte \itemId
	.endm


	.macro addpcitem itemId:req, quantity=1
	.byte 0x49
	.2byte \itemId
	.2byte \quantity
	.endm


	.macro checkpcitem itemId:req, quantity=1
	.byte 0x4a
	.2byte \itemId
	.2byte \quantity
	.endm


	.macro adddecoration decoration:req
	.byte 0x4b
	.2byte \decoration
	.endm


	.macro removedecoration decoration:req
	.byte 0x4c
	.2byte \decoration
	.endm


	.macro checkdecor decoration:req
	.byte 0x4d
	.2byte \decoration
	.endm


	.macro checkdecorspace decoration:req
	.byte 0x4e
	.2byte \decoration
	.endm


	.macro applymovement localId:req, movements:req, map
		.ifb \map
			.byte 0x4f
			.2byte \localId
			.4byte \movements
		.else
		
			.byte 0x50
			.2byte \localId
			.4byte \movements
			map \map
		.endif
	.endm





	.macro waitmovement localId=LOCALID_NONE, map
		.ifb \map
			.byte 0x51
			.2byte \localId
		.else
			.byte 0x52
			.2byte \localId
			map \map
		.endif
	.endm




	.macro removeobject localId:req, map
		.ifb \map
			.byte 0x53
			.2byte \localId
		.else
			.byte 0x54
			.2byte \localId
			map \map
		.endif
	.endm




	.macro addobject localId:req, map
		.ifb \map
			.byte 0x55
			.2byte \localId
		.else
			.byte 0x56
			.2byte \localId
			map \map
		.endif
	.endm


	.macro setobjectxy localId:req, x:req, y:req
	.byte 0x57
	.2byte \localId
	.2byte \x
	.2byte \y
	.endm


	.macro showobjectat localId:req, map:req
	.byte 0x58
	.2byte \localId
	map \map
	.endm


	.macro hideobjectat localId:req, map:req
	.byte 0x59
	.2byte \localId
	map \map
	.endm


	.macro faceplayer
	.byte 0x5a
	.endm


	.macro turnobject localId:req, direction:req
	.byte 0x5b
	.2byte \localId
	.byte \direction
	.endm


	.macro trainerbattle type:req, trainer:req, local_id:req, pointer1:req, pointer2, pointer3, pointer4
		.byte 0x5c
		.byte \type
		.2byte \trainer
		.2byte \local_id
		.if \type == TRAINER_BATTLE_SINGLE
			.4byte \pointer1
			.4byte \pointer2
		.elseif \type == TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC
			.4byte \pointer1
			.4byte \pointer2
			.4byte \pointer3
		.elseif \type == TRAINER_BATTLE_CONTINUE_SCRIPT
			.4byte \pointer1
			.4byte \pointer2
			.4byte \pointer3
		.elseif \type == TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT
			.4byte \pointer1
		.elseif \type == TRAINER_BATTLE_DOUBLE
			.4byte \pointer1
			.4byte \pointer2
			.4byte \pointer3
		.elseif \type == TRAINER_BATTLE_REMATCH
			.4byte \pointer1
			.4byte \pointer2
		.elseif \type == TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE
			.4byte \pointer1
			.4byte \pointer2
			.4byte \pointer3
			.4byte \pointer4
		.elseif \type == TRAINER_BATTLE_REMATCH_DOUBLE
			.4byte \pointer1
			.4byte \pointer2
			.4byte \pointer3
		.elseif \type == TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE_NO_MUSIC
			.4byte \pointer1
			.4byte \pointer2
			.4byte \pointer3
			.4byte \pointer4
		.elseif \type == TRAINER_BATTLE_EARLY_RIVAL
			.4byte \pointer1
			.4byte \pointer2
		.endif
	.endm

	NO_MUSIC = FALSE



	.macro trainerbattle_single trainer:req, intro_text:req, lose_text:req, event_script=FALSE, music=TRUE
	.if \event_script == FALSE
	trainerbattle TRAINER_BATTLE_SINGLE, \trainer, 0, \intro_text, \lose_text
	.elseif \music == TRUE
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, \trainer, 0, \intro_text, \lose_text, \event_script
	.else
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, \trainer, 0, \intro_text, \lose_text, \event_script
	.endif
	.endm



	.macro trainerbattle_double trainer:req, intro_text:req, lose_text:req, not_enough_pkmn_text:req, event_script=FALSE, music=TRUE
	.if \event_script == FALSE
	trainerbattle TRAINER_BATTLE_DOUBLE, \trainer, 0, \intro_text, \lose_text, \not_enough_pkmn_text
	.elseif \music == TRUE
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE, \trainer, 0, \intro_text, \lose_text, \not_enough_pkmn_text, \event_script
	.else
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE_NO_MUSIC, \trainer, 0, \intro_text, \lose_text, \not_enough_pkmn_text, \event_script
	.endif
	.endm


	.macro trainerbattle_rematch trainer:req, intro_text:req, lose_text:req
	trainerbattle TRAINER_BATTLE_REMATCH, \trainer, 0, \intro_text, \lose_text
	.endm


	.macro trainerbattle_rematch_double trainer:req, intro_text:req, lose_text:req, not_enough_pkmn_text:req
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, \trainer, 0, \intro_text, \lose_text, \not_enough_pkmn_text
	.endm


	.macro trainerbattle_no_intro trainer:req, lose_text:req
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, \trainer, 0, \lose_text
	.endm


	.macro trainerbattle_earlyrival trainer:req, flags:req, defeat_text:req, victory_text:req
	trainerbattle TRAINER_BATTLE_EARLY_RIVAL, \trainer, \flags, \defeat_text, \victory_text
	.endm



	.macro dotrainerbattle
	.byte 0x5d
	.endm


	.macro gotopostbattlescript
	.byte 0x5e
	.endm


	.macro gotobeatenscript
	.byte 0x5f
	.endm


	.macro checktrainerflag trainer:req
	.byte 0x60
	.2byte \trainer
	.endm


	.macro settrainerflag trainer:req
	.byte 0x61
	.2byte \trainer
	.endm


	.macro cleartrainerflag trainer:req
	.byte 0x62
	.2byte \trainer
	.endm



	.macro setobjectxyperm localId:req, x:req, y:req
	.byte 0x63
	.2byte \localId
	.2byte \x
	.2byte \y
	.endm



	.macro copyobjectxytoperm localId:req
	.byte 0x64
	.2byte \localId
	.endm


	.macro setobjectmovementtype localId:req, movementType:req
	.byte 0x65
	.2byte \localId
	.byte \movementType
	.endm



	.macro waitmessage
	.byte 0x66
	.endm




	.macro message text:req
	.byte 0x67
	.4byte \text
	.endm


	.macro closemessage
	.byte 0x68
	.endm


	.macro lockall
	.byte 0x69
	.endm


	.macro lock
	.byte 0x6a
	.endm


	.macro releaseall
	.byte 0x6b
	.endm


	.macro release
	.byte 0x6c
	.endm


	.macro waitbuttonpress
	.byte 0x6d
	.endm



	.macro yesnobox x:req, y:req
	.byte 0x6e
	.byte \x
	.byte \y
	.endm




	.macro multichoice x:req, y:req, multichoiceId:req, ignoreBPress:req
	.byte 0x6f
	.byte \x
	.byte \y
	.byte \multichoiceId
	.byte \ignoreBPress
	.endm





	.macro multichoicedefault x:req, y:req, multichoiceId:req, default:req, ignoreBPress:req
	.byte 0x70
	.byte \x
	.byte \y
	.byte \multichoiceId
	.byte \default
	.byte \ignoreBPress
	.endm





	.macro multichoicegrid x:req, y:req, multichoiceId:req, per_row:req, ignoreBPress:req
	.byte 0x71
	.byte \x
	.byte \y
	.byte \multichoiceId
	.byte \per_row
	.byte \ignoreBPress
	.endm


	.macro drawbox
	.byte 0x72
	.endm


	.macro erasebox left:req, top:req, right:req, bottom:req
	.byte 0x73
	.byte \left
	.byte \top
	.byte \right
	.byte \bottom
	.endm


	.macro drawboxtext left:req, top:req, multichoiceId:req, ignoreBPress:req
	.byte 0x74
	.byte \left
	.byte \top
	.byte \multichoiceId
	.byte \ignoreBPress
	.endm


	.macro showmonpic species:req, x:req, y:req
	.byte 0x75
	.2byte \species
	.byte \x
	.byte \y
	.endm


	.macro hidemonpic
	.byte 0x76
	.endm


	.macro showcontestpainting winnerId:req
	.byte 0x77
	.byte \winnerId
	.endm




	.macro braillemessage text:req
	.byte 0x78
	.4byte \text
	.endm


	.macro brailleformat winLeft:req, winTop:req, winRight:req, winBottom:req, textLeft:req, textTop:req
	.byte \winLeft
	.byte \winTop
	.byte \winRight
	.byte \winBottom
	.byte \textLeft
	.byte \textTop
	.endm



	.macro givemon species:req, level:req, item=ITEM_NONE
	.byte 0x79
	.2byte \species
	.byte \level
	.2byte \item
	.4byte 0
	.4byte 0
	.byte 0
	.endm



	.macro giveegg species:req
	.byte 0x7a
	.2byte \species
	.endm




	.macro setmonmove partyIndex:req, slot:req, move:req
	.byte 0x7b
	.byte \partyIndex
	.byte \slot
	.2byte \move
	.endm




	.macro checkpartymove move:req
	.byte 0x7c
	.2byte \move
	.endm





	.macro stringvar id:req
	.if \id == STR_VAR_1
		.byte 0
	.elseif \id == STR_VAR_2
		.byte 1
	.elseif \id == STR_VAR_3
		.byte 2
	.else
		.byte \id
	.endif
	.endm


	.macro bufferspeciesname stringVarId:req, species:req
	.byte 0x7d
	stringvar \stringVarId
	.2byte \species
	.endm


	.macro bufferleadmonspeciesname stringVarId:req
	.byte 0x7e
	stringvar \stringVarId
	.endm



	.macro bufferpartymonnick stringVarId:req, slot:req
	.byte 0x7f
	stringvar \stringVarId
	.2byte \slot
	.endm



	.macro bufferitemname stringVarId:req, item:req
	.byte 0x80
	stringvar \stringVarId
	.2byte \item
	.endm


	.macro bufferdecorationname stringVarId:req, decoration:req
	.byte 0x81
	stringvar \stringVarId
	.2byte \decoration
	.endm


	.macro buffermovename stringVarId:req, move:req
	.byte 0x82
	stringvar \stringVarId
	.2byte \move
	.endm


	.macro buffernumberstring stringVarId:req, input:req
	.byte 0x83
	stringvar \stringVarId
	.2byte \input
	.endm


	.macro bufferstdstring stringVarId:req, index:req
	.byte 0x84
	stringvar \stringVarId
	.2byte \index
	.endm


	.macro bufferstring stringVarId:req, text:req
	.byte 0x85
	stringvar \stringVarId
	.4byte \text
	.endm



	.macro pokemart products:req
	.byte 0x86
	.4byte \products
	.endm



	.macro pokemartdecoration products:req
	.byte 0x87
	.4byte \products
	.endm


	.macro pokemartdecoration2 products:req
	.byte 0x88
	.4byte \products
	.endm


	.macro playslotmachine id:req
	.byte 0x89
	.2byte \id
	.endm


	.macro setberrytree treeId:req, berry:req, growthStage:req
	.byte 0x8a
	.byte \treeId
	.byte \berry
	.byte \growthStage
	.endm


	.macro choosecontestmon
	.byte 0x8b
	.endm


	.macro startcontest
	.byte 0x8c
	.endm


	.macro showcontestresults
	.byte 0x8d
	.endm


	.macro contestlinktransfer
	.byte 0x8e
	.endm


	.macro random limit:req
	.byte 0x8f
	.2byte \limit
	.endm



	.macro addmoney value:req, disable=0
	.byte 0x90
	.4byte \value
	.byte \disable
	.endm



	.macro removemoney value:req, disable=0
	.byte 0x91
	.4byte \value
	.byte \disable
	.endm



	.macro checkmoney value:req, disable=0
	.byte 0x92
	.4byte \value
	.byte \disable
	.endm



	.macro showmoneybox x:req, y:req, disable=0
	.byte 0x93
	.byte \x
	.byte \y
	.byte \disable
	.endm


	.macro hidemoneybox
	.byte 0x94
	.byte 0 
	.byte 0 
	.endm



	.macro updatemoneybox disable=0
	.byte 0x95
	.byte 0 
	.byte 0 
	.byte \disable
	.endm


	.macro getpokenewsactive newsKind:req
	.byte 0x96
	.2byte \newsKind
	.endm


	.macro fadescreen mode:req
	.byte 0x97
	.byte \mode
	.endm


	.macro fadescreenspeed mode:req, speed:req
	.byte 0x98
	.byte \mode
	.byte \speed
	.endm



	.macro setflashlevel level:req
	.byte 0x99
	.2byte \level
	.endm



	.macro animateflash level:req
	.byte 0x9a
	.byte \level
	.endm


	.macro messageautoscroll text:req
	.byte 0x9b
	.4byte \text
	.endm


	.macro dofieldeffect animation:req
	.byte 0x9c
	.2byte \animation
	.endm


	.macro setfieldeffectargument argNum:req, value:req
	.byte 0x9d
	.byte \argNum
	.2byte \value
	.endm


	.macro waitfieldeffect animation:req
	.byte 0x9e
	.2byte \animation
	.endm


	.macro setrespawn heallocation:req
	.byte 0x9f
	.2byte \heallocation
	.endm


	.macro checkplayergender
	.byte 0xa0
	.endm



	.macro playmoncry species:req, mode:req
	.byte 0xa1
	.2byte \species
	.2byte \mode
	.endm


	.macro setmetatile x:req, y:req, metatileId:req, impassable:req
	.byte 0xa2
	.2byte \x
	.2byte \y
	.2byte \metatileId
	.2byte \impassable
	.endm


	.macro resetweather
	.byte 0xa3
	.endm


	.macro setweather type:req
	.byte 0xa4
	.2byte \type
	.endm


	.macro doweather
	.byte 0xa5
	.endm


	.macro setstepcallback stepCbId:req
	.byte 0xa6
	.byte \stepCbId
	.endm



	.macro setmaplayoutindex index:req
	.byte 0xa7
	.2byte \index
	.endm



	.macro setobjectsubpriority localId:req, map:req, subpriority:req
	.byte 0xa8
	.2byte \localId
	map \map
	.byte \subpriority
	.endm


	.macro resetobjectsubpriority localId:req, map:req
	.byte 0xa9
	.2byte \localId
	map \map
	.endm



	.macro createvobject graphicsId:req, id:req, x:req, y:req, elevation=3, direction=DIR_SOUTH
	.byte 0xaa
	.byte \graphicsId
	.byte \id
	.2byte \x
	.2byte \y
	.byte \elevation
	.byte \direction
	.endm


	.macro turnvobject id:req, direction:req
	.byte 0xab
	.byte \id
	.byte \direction
	.endm


	.macro opendoor x:req, y:req
	.byte 0xac
	.2byte \x
	.2byte \y
	.endm


	.macro closedoor x:req, y:req
	.byte 0xad
	.2byte \x
	.2byte \y
	.endm


	.macro waitdooranim
	.byte 0xae
	.endm


	.macro setdooropen x:req, y:req
	.byte 0xaf
	.2byte \x
	.2byte \y
	.endm


	.macro setdoorclosed x:req, y:req
	.byte 0xb0
	.2byte \x
	.2byte \y
	.endm


	.macro addelevmenuitem a:req, b:req, c:req, d:req
	.byte 0xb1
	.byte \a
	.2byte \b
	.2byte \c
	.2byte \d
	.endm


	.macro showelevmenu
	.byte 0xb2
	.endm


	.macro checkcoins out:req
	.byte 0xb3
	.2byte \out
	.endm



	.macro addcoins count:req
	.byte 0xb4
	.2byte \count
	.endm




	.macro removecoins count:req
	.byte 0xb5
	.2byte \count
	.endm



	.macro setwildbattle species:req, level:req, item=ITEM_NONE
	.byte 0xb6
	.2byte \species
	.byte \level
	.2byte \item
	.endm


	.macro dowildbattle
	.byte 0xb7
	.endm


	.macro setvaddress pointer:req
	.byte 0xb8
	.4byte \pointer
	.endm


	.macro vgoto destination:req
	.byte 0xb9
	.4byte \destination
	.endm


	.macro vcall destination:req
	.byte 0xba
	.4byte \destination
	.endm


	.macro vgoto_if condition:req, destination:req
	.byte 0xbb
	.byte \condition
	.4byte \destination
	.endm


	.macro vcall_if condition:req, destination:req
	.byte 0xbc
	.byte \condition
	.4byte \destination
	.endm


	.macro vmessage text:req
	.byte 0xbd
	.4byte \text
	.endm


	.macro vbuffermessage text:req
	.byte 0xbe
	.4byte \text
	.endm


	.macro vbufferstring stringVarIndex:req, text:req
	.byte 0xbf
	stringvar \stringVarIndex
	.4byte \text
	.endm


	.macro showcoinsbox x:req, y:req
	.byte 0xc0
	.byte \x
	.byte \y
	.endm


	.macro hidecoinsbox x:req, y:req
	.byte 0xc1
	.byte \x
	.byte \y
	.endm


	.macro updatecoinsbox x:req, y:req
	.byte 0xc2
	.byte \x
	.byte \y
	.endm


	.macro incrementgamestat stat:req
	.byte 0xc3
	.byte \stat
	.endm




	.macro setescapewarp map:req, a, b, c
	.byte 0xc4
	formatwarp \map, \a, \b, \c
	.endm


	.macro waitmoncry
	.byte 0xc5
	.endm


	.macro bufferboxname stringVarId:req, box:req
	.byte 0xc6
	stringvar \stringVarId
	.2byte \box
	.endm



	.macro textcolor color:req
	.byte 0xc7
	.byte \color
	.endm


	.macro loadhelp msg:req
	.byte 0xc8
	.4byte \msg
	.endm


	.macro unloadhelp
	.byte 0xc9
	.endm


	.macro signmsg
	.byte 0xca
	.endm


	.macro normalmsg
	.byte 0xcb
	.endm



	.macro comparestat statId:req, value:req
	.byte 0xcc
	.byte \statId
	.4byte \value
	.endm


	.macro setmonmodernfatefulencounter slot:req
	.byte 0xcd
	.2byte \slot
	.endm



	.macro checkmonmodernfatefulencounter slot:req
	.byte 0xce
	.2byte \slot
	.endm



	.macro trywondercardscript
	.byte 0xcf
	.endm


	.macro setworldmapflag worldmapflag:req
	.byte 0xd0
	.2byte \worldmapflag
	.endm





	.macro warpspinenter map:req, a, b, c
	.byte 0xd1
	formatwarp \map, \a, \b, \c
	.endm


	.macro setmonmetlocation slot:req, location:req
	.byte 0xd2
	.2byte \slot
	.byte \location
	.endm


	.macro getbraillestringwidth msg:req
	.byte 0xd3
	.4byte \msg
	.endm




	.macro bufferitemnameplural stringVarId:req, item:req, quantity:req
	.byte 0xd4
	stringvar \stringVarId
	.2byte \item
	.2byte \quantity
	.endm




	.macro goto_if_unset flag:req, dest:req
	checkflag \flag
	goto_if FALSE, \dest
	.endm

	.macro goto_if_set flag:req, dest:req
	checkflag \flag
	goto_if TRUE, \dest
	.endm












	.macro trycompare jump:req, condition:req, a:req, b, c
	.ifnb \c
		compare \a, \b
		\jump \condition, \c
	.else
		\jump \condition, \a
	.endif
	.endm

	.macro goto_if_lt a:req, b, c
	trycompare goto_if, 0, \a, \b, \c
	.endm

	.macro goto_if_eq a:req, b, c
	trycompare goto_if, 1, \a, \b, \c
	.endm

	.macro goto_if_gt a:req, b, c
	trycompare goto_if, 2, \a, \b, \c
	.endm

	.macro goto_if_le a:req, b, c
	trycompare goto_if, 3, \a, \b, \c
	.endm

	.macro goto_if_ge a:req, b, c
	trycompare goto_if, 4, \a, \b, \c
	.endm

	.macro goto_if_ne a:req, b, c
	trycompare goto_if, 5, \a, \b, \c
	.endm

	.macro call_if_unset flag:req, dest:req
	checkflag \flag
	call_if FALSE, \dest
	.endm

	.macro call_if_set flag:req, dest:req
	checkflag \flag
	call_if TRUE, \dest
	.endm

	.macro call_if_lt a:req, b, c
	trycompare call_if, 0, \a, \b, \c
	.endm

	.macro call_if_eq a:req, b, c
	trycompare call_if, 1, \a, \b, \c
	.endm

	.macro call_if_gt a:req, b, c
	trycompare call_if, 2, \a, \b, \c
	.endm

	.macro call_if_le a:req, b, c
	trycompare call_if, 3, \a, \b, \c
	.endm

	.macro call_if_ge a:req, b, c
	trycompare call_if, 4, \a, \b, \c
	.endm

	.macro call_if_ne a:req, b, c
	trycompare call_if, 5, \a, \b, \c
	.endm

	.macro vgoto_if_eq a:req, b, c
	trycompare vgoto_if, TRUE, \a, \b, \c
	.endm

	.macro vgoto_if_ne a:req, b, c
	trycompare vgoto_if, FALSE, \a, \b, \c
	.endm

	.macro vgoto_if_set flag:req, dest:req
	checkflag \flag
	vgoto_if TRUE, \dest
	.endm

	.macro vgoto_if_unset flag:req, dest:req
	checkflag \flag
	vgoto_if FALSE, \dest
	.endm

	.macro goto_if_defeated trainer:req, dest:req
	checktrainerflag \trainer
	goto_if TRUE, \dest
	.endm

	.macro goto_if_not_defeated trainer:req, dest:req
	checktrainerflag \trainer
	goto_if FALSE, \dest
	.endm

	.macro call_if_defeated trainer:req, dest:req
	checktrainerflag \trainer
	call_if TRUE, \dest
	.endm

	.macro call_if_not_defeated trainer:req, dest:req
	checktrainerflag \trainer
	call_if FALSE, \dest
	.endm

	.macro switch var:req
	copyvar VAR_0x8000, \var
	.endm

	.macro case condition:req, dest:req
	compare VAR_0x8000, \condition
	goto_if_eq \dest
	.endm

	YES = 1
	NO  = 0


	.macro msgbox text:req, type=MSGBOX_DEFAULT
	loadword 0, \text
	callstd \type
	.endm





	.macro giveitem item:req, amount=1
	setorcopyvar VAR_0x8000, \item
	setorcopyvar VAR_0x8001, \amount
	callstd STD_OBTAIN_ITEM
	.endm



	.macro finditem item:req, amount=1
	setorcopyvar VAR_0x8000, \item
	setorcopyvar VAR_0x8001, \amount
	callstd STD_FIND_ITEM
	.endm


	.macro putitemaway item:req, amount=1
	setorcopyvar VAR_0x8000, \item
	setorcopyvar VAR_0x8001, \amount
	callstd STD_PUT_ITEM_AWAY
	.endm



	.macro msgreceiveditem msg:req, item:req, amount=1, fanfare=MUS_LEVEL_UP
	loadword 0, \msg
	setorcopyvar VAR_0x8000, \item
	setorcopyvar VAR_0x8001, \amount
	setorcopyvar VAR_0x8002, \fanfare
	callstd STD_RECEIVED_ITEM
	.endm


	.macro giveitem_msg msg:req, item:req, amount=1, fanfare=MUS_LEVEL_UP
	additem \item, \amount
	msgreceiveditem \msg, \item, \amount, \fanfare
	.endm


	.macro famechecker person:req, index:req, function=SetFlavorTextFlagFromSpecialVars
	setvar VAR_0x8004, \person
	setvar VAR_0x8005, \index
	special \function
	.endm


	.macro set_gym_trainers gym:req
	setvar VAR_0x8008, \gym
	call EventScript_SetGymTrainers
	.endm


	.macro goto_if_questlog dest:req
	special GetQuestLogState
	compare VAR_RESULT, 2
	goto_if_eq \dest
	.endm


	.macro braillemessage_wait text:req
	setvar VAR_0x8006, 0
	braillemessage \text
	getbraillestringwidth \text
	call EventScript_BrailleCursorWaitButton
	.endm


	.macro seteventmon species:req, level:req, item=ITEM_NONE
	setvar VAR_0x8004, \species
	setvar VAR_0x8005, \level
	setvar VAR_0x8006, \item
	special CreateEnemyEventMon
	.endm
