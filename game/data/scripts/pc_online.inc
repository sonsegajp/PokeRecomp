@ PC port: online play. The Pokémon Center 2F desk on the left is the
@ ONLINE CLUB (server browser). Once on a server, talking to another trainer
@ offers a battle or trade; both games then run the original Cable Club
@ link-up and rooms over the network (see src/pc_online.c, src/link.c).

Online_EventScript_Attendant::
	goto_if_questlog EventScript_ReleaseEnd
	special QuestLog_CutRecording
	lock
	faceplayer
	msgbox Online_Text_Welcome
	fadescreen FADE_TO_BLACK
	special Online_OpenScreen
	waitstate
	goto_if_eq VAR_RESULT, TRUE, Online_EventScript_AttendantJoined
	msgbox Online_Text_ComeAgain
	release
	end

Online_EventScript_AttendantJoined::
	release
	end

@ On arrival in Pallet Town after joining a server.
Online_EventScript_JoinedMessage::
	lock
	special Online_BufferServerName
	msgbox Online_Text_Joined
	release
	end

@ The old local wireless / link cable desks.
Online_EventScript_LocalClosed::
	goto_if_questlog EventScript_ReleaseEnd
	lock
	faceplayer
	msgbox Online_Text_LocalClosed
	release
	end

Online_EventScript_TalkToPlayer::
	lock
	special Online_BufferTarget
	message Online_Text_WhatToDo
	waitmessage
	multichoice 19, 6, MULTICHOICE_ONLINE_PLAYER, FALSE
	switch VAR_RESULT
	case 0, Online_EventScript_AskBattle
	case 1, Online_EventScript_AskTrade
	goto Online_EventScript_End

Online_EventScript_AskBattle::
	setvar VAR_0x8004, 1
	goto Online_EventScript_Ask

Online_EventScript_AskTrade::
	call CableClub_EventScript_CheckPartyTradeRequirements
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_End
	setvar VAR_0x8004, 3
	goto Online_EventScript_Ask

Online_EventScript_Ask::
	call EventScript_AskSaveGame
	goto_if_eq VAR_RESULT, 0, Online_EventScript_End
	special Online_BufferTarget
	message Online_Text_Waiting
	waitmessage
	special Online_SendRequest
	waitstate
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_Declined
	goto Online_EventScript_StartLink

Online_EventScript_Declined::
	msgbox Online_Text_Declined
	goto Online_EventScript_End

Online_EventScript_End::
	closemessage
	release
	end

@ Started by the game when another trainer asks (VAR_0x8006 = kind,
@ STR_VAR_1 = their name).
Online_EventScript_IncomingRequest::
	lock
	playse SE_PIN
	goto_if_eq VAR_0x8006, 3, Online_EventScript_IncomingTrade
	msgbox Online_Text_WantsBattle, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_Decline
	goto Online_EventScript_IncomingAccept

Online_EventScript_IncomingTrade::
	msgbox Online_Text_WantsTrade, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_Decline
	call CableClub_EventScript_CheckPartyTradeRequirements
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_Decline
Online_EventScript_IncomingAccept::
	call EventScript_AskSaveGame
	goto_if_eq VAR_RESULT, 0, Online_EventScript_Decline
	message Online_Text_Connecting
	waitmessage
	special Online_AcceptRequest
	waitstate
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_LinkFailed
	goto Online_EventScript_StartLink

Online_EventScript_Decline::
	special Online_DeclineRequest
	goto Online_EventScript_End

@ Both games are now on a link session: the Cable Club link-up, then the room.
Online_EventScript_StartLink::
	special Online_SessionKind
	goto_if_eq VAR_RESULT, 3, Online_EventScript_LinkTrade
	setvar VAR_0x8004, USING_SINGLE_BATTLE
	special HelpSystem_Disable
	special TryBattleLinkup
	waitstate
	goto_if_ne VAR_RESULT, LINKUP_SUCCESS, Online_EventScript_LinkFailed
	special HealPlayerParty
	special SavePlayerParty
	special LoadPlayerBag
	closemessage
	special Online_StartBattle
	waitstate
	end

Online_EventScript_LinkTrade::
	special HelpSystem_Disable
	special TryTradeLinkup
	waitstate
	goto_if_ne VAR_RESULT, LINKUP_SUCCESS, Online_EventScript_LinkFailed
	setvar VAR_0x8004, USING_TRADE_CENTER
	closemessage
	special Online_StartTrade
	waitstate
	end

@ Test hooks (--net-request / --net-accept): the same path without menus.
Online_EventScript_DebugRequest::
	lock
	special Online_SendRequest
	waitstate
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_End
	goto Online_EventScript_StartLink

Online_EventScript_DebugAccept::
	lock
	special Online_AcceptRequest
	waitstate
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_LinkFailed
	goto Online_EventScript_StartLink

Online_EventScript_LinkFailed::
	special CloseLink
	special HelpSystem_Enable
	msgbox Online_Text_LinkFailed
	goto Online_EventScript_End

Online_Text_Welcome::
	.string "Welcome to the POKéMON ONLINE CLUB!\p"
	.string "Pick a server to meet, battle and\n"
	.string "trade with trainers everywhere.$"

Online_Text_ComeAgain::
	.string "Please come again!$"

Online_Text_Joined::
	.string "You're now on {STR_VAR_1}!\p"
	.string "Trainers you see around KANTO are\n"
	.string "real players. Talk to one to battle\l"
	.string "or trade!$"

Online_Text_LocalClosed::
	.string "Local link play has moved online.\p"
	.string "Please visit the ONLINE CLUB desk\n"
	.string "on the left!$"

Online_Text_WhatToDo::
	.string "It's {STR_VAR_1}!\n"
	.string "What would you like to do?$"

Online_Text_Waiting::
	.string "Waiting for {STR_VAR_1}…\n"
	.string "Please wait.$"

Online_Text_Declined::
	.string "{STR_VAR_1} can't right now.$"

Online_Text_WantsBattle::
	.string "{STR_VAR_1} wants to battle!\n"
	.string "Accept the challenge?$"

Online_Text_WantsTrade::
	.string "{STR_VAR_1} wants to trade!\n"
	.string "Accept?$"

Online_Text_Connecting::
	.string "Connecting…$"

Online_Text_LinkFailed::
	.string "The connection was lost.$"
