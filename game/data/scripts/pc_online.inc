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
	multichoice 19, 4, MULTICHOICE_ONLINE_PLAYER, FALSE
	switch VAR_RESULT
	case 0, Online_EventScript_AskBattle
	case 1, Online_EventScript_AskTrade
	case 2, Online_EventScript_AddFriend
	goto Online_EventScript_End

Online_EventScript_AddFriend::
	message Online_Text_Connecting
	waitmessage
	special Online_AddFriend
	waitstate
	special Online_BufferTarget
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_AddFriendFailed
	msgbox Online_Text_FriendAdded
	release
	end

Online_EventScript_AddFriendFailed::
	msgbox Online_Text_GlobalFailed
	release
	end

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

@ ---- GLOBAL TRADE desk (the middle 2F desk): Wonder Trade, GTS, mailbox ----

Online_EventScript_GlobalTrade::
	goto_if_questlog EventScript_ReleaseEnd
	special QuestLog_CutRecording
	lock
	faceplayer
	special Online_HasAccount
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_GlobalNeedAccount
	message Online_Text_GlobalWelcome
	waitmessage
	multichoice 17, 0, MULTICHOICE_GLOBAL_TRADE, FALSE
	switch VAR_RESULT
	case 0, Online_EventScript_Wonder
	case 1, Online_EventScript_Gts
	case 2, Online_EventScript_Mailbox
	goto Online_EventScript_GlobalBye

Online_EventScript_GlobalNeedAccount::
	msgbox Online_Text_GlobalNeedAccount
	release
	end

Online_EventScript_GlobalBye::
	msgbox Online_Text_ComeAgain
	release
	end

Online_EventScript_Wonder::
	msgbox Online_Text_WonderChoose
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Online_EventScript_GlobalBye
	specialvar VAR_RESULT, GetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, Online_EventScript_WonderEgg
	specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
	goto_if_eq VAR_RESULT, 0, Online_EventScript_WonderLast
	special BufferMonNickname
	msgbox Online_Text_WonderConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_GlobalBye
	call EventScript_AskSaveGame
	goto_if_eq VAR_RESULT, 0, Online_EventScript_GlobalBye
	message Online_Text_Connecting
	waitmessage
	special Online_WonderTrade
	waitstate
	switch VAR_RESULT
	case 1, Online_EventScript_WonderGot
	case 2, Online_EventScript_WonderWaiting
	case 3, Online_EventScript_GlobalIllegal
	goto Online_EventScript_GlobalFailed

Online_EventScript_WonderGot::
	playfanfare MUS_OBTAIN_ITEM
	msgbox Online_Text_WonderGot
	waitfanfare
	release
	end

Online_EventScript_WonderWaiting::
	msgbox Online_Text_WonderWaiting
	release
	end

Online_EventScript_WonderEgg::
	msgbox Online_Text_WonderEgg
	release
	end

Online_EventScript_WonderLast::
	msgbox Online_Text_WonderLast
	release
	end

Online_EventScript_GlobalIllegal::
	msgbox Online_Text_GlobalIllegal
	release
	end

Online_EventScript_GlobalFailed::
	msgbox Online_Text_GlobalFailed
	release
	end

Online_EventScript_Mailbox::
	setvar VAR_0x8007, 0
Online_EventScript_MailboxNext::
	message Online_Text_Checking
	waitmessage
	special Online_CollectMail
	waitstate
	switch VAR_RESULT
	case 1, Online_EventScript_MailboxGot
	case 2, Online_EventScript_MailboxEmpty
	case 4, Online_EventScript_MailboxNoRoom
	goto Online_EventScript_GlobalFailed

Online_EventScript_MailboxGot::
	addvar VAR_0x8007, 1
	playfanfare MUS_OBTAIN_ITEM
	msgbox Online_Text_MailboxGot
	waitfanfare
	goto_if_eq VAR_0x8005, 0, Online_EventScript_MailboxNext
	msgbox Online_Text_MailboxToPC
	goto Online_EventScript_MailboxNext

Online_EventScript_MailboxEmpty::
	goto_if_ne VAR_0x8007, 0, Online_EventScript_MailboxDone
	msgbox Online_Text_MailboxEmpty
	release
	end

Online_EventScript_MailboxDone::
	msgbox Online_Text_MailboxDone
	release
	end

Online_EventScript_MailboxNoRoom::
	msgbox Online_Text_MailboxNoRoom
	release
	end

Online_EventScript_Gts::
	message Online_Text_GtsWhat
	waitmessage
	multichoice 17, 0, MULTICHOICE_GTS, FALSE
	switch VAR_RESULT
	case 0, Online_EventScript_GtsDeposit
	case 1, Online_EventScript_GtsSearch
	case 2, Online_EventScript_GtsMine
	goto Online_EventScript_GlobalBye

@ DEPOSIT: party Pokémon -> the GTS screen (what you want for it) -> save -> listed
Online_EventScript_GtsDeposit::
	msgbox Online_Text_GtsDepositChoose
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Online_EventScript_GlobalBye
	specialvar VAR_RESULT, GetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, Online_EventScript_GtsEgg
	specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
	goto_if_eq VAR_RESULT, 0, Online_EventScript_WonderLast
	setvar VAR_0x8006, 0
	fadescreen FADE_TO_BLACK
	special Online_OpenGts
	waitstate
	goto_if_eq VAR_RESULT, 0, Online_EventScript_GlobalBye
	special BufferMonNickname
	msgbox Online_Text_GtsDepositConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_GlobalBye
	call EventScript_AskSaveGame
	goto_if_eq VAR_RESULT, 0, Online_EventScript_GlobalBye
	message Online_Text_Connecting
	waitmessage
	special Online_GtsDeposit
	waitstate
	switch VAR_RESULT
	case 1, Online_EventScript_GtsDeposited
	case 3, Online_EventScript_GlobalIllegal
	case 4, Online_EventScript_GtsFull
	goto Online_EventScript_GlobalFailed

Online_EventScript_GtsDeposited::
	playse SE_SUCCESS
	msgbox Online_Text_GtsDeposited
	release
	end

Online_EventScript_GtsFull::
	msgbox Online_Text_GtsFull
	release
	end

Online_EventScript_GtsEgg::
	msgbox Online_Text_GtsEgg
	release
	end

@ SEARCH: the GTS screen (species -> listings) -> offer a party Pokémon that fits
Online_EventScript_GtsSearch::
	setvar VAR_0x8006, 1
	fadescreen FADE_TO_BLACK
	special Online_OpenGts
	waitstate
	goto_if_eq VAR_RESULT, 0, Online_EventScript_GlobalBye
	msgbox Online_Text_GtsOfferChoose
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Online_EventScript_GlobalBye
	special Online_GtsCheckOffer
	switch VAR_RESULT
	case 1, Online_EventScript_GtsWrongSpecies
	case 2, Online_EventScript_GtsWrongLevel
	case 3, Online_EventScript_GtsWrongGender
	msgbox Online_Text_GtsOfferConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_GlobalBye
	call EventScript_AskSaveGame
	goto_if_eq VAR_RESULT, 0, Online_EventScript_GlobalBye
	message Online_Text_Connecting
	waitmessage
	special Online_GtsOffer
	waitstate
	switch VAR_RESULT
	case 1, Online_EventScript_WonderGot
	case 3, Online_EventScript_GlobalIllegal
	case 5, Online_EventScript_GtsGone
	case 6, Online_EventScript_GtsWrongSpecies
	goto Online_EventScript_GlobalFailed

Online_EventScript_GtsWrongSpecies::
	msgbox Online_Text_GtsWrongSpecies
	release
	end

Online_EventScript_GtsWrongLevel::
	msgbox Online_Text_GtsWrongLevel
	release
	end

Online_EventScript_GtsWrongGender::
	msgbox Online_Text_GtsWrongGender
	release
	end

Online_EventScript_GtsGone::
	msgbox Online_Text_GtsGone
	release
	end

@ MY POKéMON: the GTS screen (your listings) -> take one back
Online_EventScript_GtsMine::
	setvar VAR_0x8006, 2
	fadescreen FADE_TO_BLACK
	special Online_OpenGts
	waitstate
	goto_if_eq VAR_RESULT, 0, Online_EventScript_GlobalBye
	msgbox Online_Text_GtsTakeBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Online_EventScript_GlobalBye
	call EventScript_AskSaveGame
	goto_if_eq VAR_RESULT, 0, Online_EventScript_GlobalBye
	message Online_Text_Connecting
	waitmessage
	special Online_GtsWithdraw
	waitstate
	switch VAR_RESULT
	case 1, Online_EventScript_GtsTookBack
	case 5, Online_EventScript_GtsAlreadyTraded
	case 7, Online_EventScript_MailboxNoRoom
	goto Online_EventScript_GlobalFailed

Online_EventScript_GtsTookBack::
	playfanfare MUS_OBTAIN_ITEM
	msgbox Online_Text_MailboxGot
	waitfanfare
	goto_if_eq VAR_0x8005, 0, Online_EventScript_GlobalBye
	msgbox Online_Text_MailboxToPC
	release
	end

Online_EventScript_GtsAlreadyTraded::
	msgbox Online_Text_GtsAlreadyTraded
	release
	end

Online_Text_GtsWhat::
	.string "This is the GTS, the GLOBAL\n"
	.string "TRADE STATION. What will you do?$"

Online_Text_GtsDepositChoose::
	.string "Which POKéMON will you put on\n"
	.string "the GTS?$"

Online_Text_GtsDepositConfirm::
	.string "Put {STR_VAR_1} on the GTS and\n"
	.string "ask for {STR_VAR_2} in return?$"

Online_Text_GtsDeposited::
	.string "{STR_VAR_1} is now on the GTS!\p"
	.string "When someone trades for it, their\n"
	.string "POKéMON goes to your MAILBOX.$"

Online_Text_GtsFull::
	.string "You already have three POKéMON\n"
	.string "on the GTS.$"

Online_Text_GtsEgg::
	.string "Eggs can't be put on the GTS.$"

Online_Text_GtsOfferChoose::
	.string "To get {STR_VAR_1}, offer {STR_VAR_2}\n"
	.string "({STR_VAR_3}). Which POKéMON?$"

Online_Text_GtsOfferConfirm::
	.string "Trade your POKéMON for\n"
	.string "{STR_VAR_1}?$"

Online_Text_GtsWrongSpecies::
	.string "That trainer wants {STR_VAR_2}.$"

Online_Text_GtsWrongLevel::
	.string "That trainer wants {STR_VAR_2}\n"
	.string "at {STR_VAR_3}.$"

Online_Text_GtsWrongGender::
	.string "That trainer wants {STR_VAR_2}\n"
	.string "of the other gender.$"

Online_Text_GtsGone::
	.string "Sorry, someone else traded for\n"
	.string "that POKéMON first.$"

Online_Text_GtsTakeBack::
	.string "Take {STR_VAR_1} back from\n"
	.string "the GTS?$"

Online_Text_GtsAlreadyTraded::
	.string "{STR_VAR_1} was already traded!\n"
	.string "Check the MAILBOX.$"

Online_Text_GlobalWelcome::
	.string "Welcome to the GLOBAL TRADE desk!\n"
	.string "How may I help you?$"

Online_Text_GlobalNeedAccount::
	.string "Please register at the ONLINE CLUB\n"
	.string "desk on the left first.$"

Online_Text_WonderChoose::
	.string "Which POKéMON will you send? It will\n"
	.string "be traded with a random trainer.$"

Online_Text_WonderConfirm::
	.string "Send {STR_VAR_1} on a WONDER TRADE?$"

Online_Text_WonderGot::
	.string "{PLAYER} received {STR_VAR_1}!$"

Online_Text_WonderWaiting::
	.string "Your POKéMON is waiting for a\n"
	.string "partner.\p"
	.string "Check the MAILBOX later to collect\n"
	.string "the POKéMON you get in return.$"

Online_Text_WonderEgg::
	.string "Eggs can't be sent on a WONDER\n"
	.string "TRADE.$"

Online_Text_WonderLast::
	.string "You can't send your last POKéMON!$"

Online_Text_GlobalIllegal::
	.string "That POKéMON can't be traded.\n"
	.string "It didn't pass the check.$"

Online_Text_GlobalFailed::
	.string "The connection failed.\n"
	.string "Please try again later.$"

Online_Text_Checking::
	.string "Checking the MAILBOX…$"

Online_Text_MailboxGot::
	.string "{PLAYER} received {STR_VAR_1}!$"

Online_Text_MailboxToPC::
	.string "It was sent to the PC.$"

Online_Text_MailboxEmpty::
	.string "No POKéMON are waiting for you.$"

Online_Text_MailboxDone::
	.string "That's everything!$"

Online_Text_MailboxNoRoom::
	.string "There's no room for more POKéMON.\n"
	.string "Make space in your PC, then come back.$"

@ ---- POKéMON MARKET (the right 2F desk): trainers sell items to each other ----

Online_EventScript_Market::
	goto_if_questlog EventScript_ReleaseEnd
	special QuestLog_CutRecording
	lock
	faceplayer
	special Online_HasAccount
	goto_if_eq VAR_RESULT, FALSE, Online_EventScript_GlobalNeedAccount
	msgbox Online_Text_MarketWelcome
	fadescreen FADE_TO_BLACK
	special Online_OpenMarket
	waitstate
	msgbox Online_Text_ComeAgain
	release
	end

Online_Text_MarketWelcome::
	.string "Welcome to the POKéMON MARKET!\n"
	.string "Trainers everywhere buy and sell here.$"

Online_Text_FriendAdded::
	.string "{STR_VAR_1} is now on your\n"
	.string "FRIENDS list!$"
