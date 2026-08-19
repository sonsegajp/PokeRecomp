	.include "MPlayDef.s"

	.equ	se_m_perish_song_grp, voicegroup128
	.equ	se_m_perish_song_pri, 4
	.equ	se_m_perish_song_rev, reverb_set+50
	.equ	se_m_perish_song_mvl, 127
	.equ	se_m_perish_song_key, 0
	.equ	se_m_perish_song_tbs, 1
	.equ	se_m_perish_song_exg, 1
	.equ	se_m_perish_song_cmp, 1

	.section .rodata
	.global	se_m_perish_song
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_perish_song_1:
	.byte	KEYSH , se_m_perish_song_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 100*se_m_perish_song_tbs/2
	.byte		VOICE , 73
	.byte		BENDR , 12
	.byte		LFOS  , 40
	.byte		PAN   , c_v+7
	.byte		VOL   , 25*se_m_perish_song_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		VOL   , 29*se_m_perish_song_mvl/mxv
	.byte	W03
@ 002   ----------------------------------------
	.byte		        33*se_m_perish_song_mvl/mxv
	.byte		N48   , An3 , v112
	.byte	W03
	.byte		VOL   , 40*se_m_perish_song_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+4
	.byte	W02
@ 003   ----------------------------------------
se_m_perish_song_1_003:
	.byte		VOL   , 45*se_m_perish_song_mvl/mxv
	.byte		MOD   , 8
	.byte	W03
	.byte		VOL   , 51*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte	PEND
@ 004   ----------------------------------------
se_m_perish_song_1_004:
	.byte		VOL   , 56*se_m_perish_song_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W03
	.byte		VOL   , 62*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte	PEND
@ 005   ----------------------------------------
se_m_perish_song_1_005:
	.byte		VOL   , 72*se_m_perish_song_mvl/mxv
	.byte		PAN   , c_v-4
	.byte	W03
	.byte		VOL   , 81*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte	PEND
@ 006   ----------------------------------------
se_m_perish_song_1_006:
	.byte		VOL   , 92*se_m_perish_song_mvl/mxv
	.byte		PAN   , c_v-8
	.byte	W03
	.byte		VOL   , 100*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte	PEND
@ 007   ----------------------------------------
	.byte		PAN   , c_v-4
	.byte	W06
@ 008   ----------------------------------------
	.byte		VOL   , 82*se_m_perish_song_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W03
	.byte		VOL   , 52*se_m_perish_song_mvl/mxv
	.byte	W03
@ 009   ----------------------------------------
	.byte		        25*se_m_perish_song_mvl/mxv
	.byte		MOD   , 0
	.byte		PAN   , c_v+4
	.byte	W03
	.byte		VOL   , 29*se_m_perish_song_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+7
	.byte	W02
@ 010   ----------------------------------------
	.byte		VOL   , 33*se_m_perish_song_mvl/mxv
	.byte		N72   , Gs3 , v112
	.byte	W03
	.byte		VOL   , 40*se_m_perish_song_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+4
	.byte	W02
@ 011   ----------------------------------------
	.byte	PATT
	 .word	se_m_perish_song_1_003
@ 012   ----------------------------------------
	.byte	PATT
	 .word	se_m_perish_song_1_004
@ 013   ----------------------------------------
	.byte	PATT
	 .word	se_m_perish_song_1_005
@ 014   ----------------------------------------
	.byte	PATT
	 .word	se_m_perish_song_1_006
@ 015   ----------------------------------------
	.byte		PAN   , c_v-4
	.byte	W06
@ 016   ----------------------------------------
	.byte		        c_v+0
	.byte	W06
@ 017   ----------------------------------------
	.byte		        c_v+4
	.byte	W03
	.byte		VOL   , 87*se_m_perish_song_mvl/mxv
	.byte	W03
@ 018   ----------------------------------------
	.byte		        75*se_m_perish_song_mvl/mxv
	.byte		PAN   , c_v+7
	.byte	W03
	.byte		VOL   , 62*se_m_perish_song_mvl/mxv
	.byte	W03
@ 019   ----------------------------------------
	.byte		        48*se_m_perish_song_mvl/mxv
	.byte		PAN   , c_v+4
	.byte	W03
	.byte		VOL   , 33*se_m_perish_song_mvl/mxv
	.byte	W03
@ 020   ----------------------------------------
	.byte		        25*se_m_perish_song_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W02
	.byte		VOL   , 10*se_m_perish_song_mvl/mxv
	.byte	W03
@ 021   ----------------------------------------
	.byte	W01
	.byte		PAN   , c_v-4
	.byte	W05
@ 022   ----------------------------------------
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_m_perish_song_2:
	.byte	KEYSH , se_m_perish_song_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 73
	.byte		VOL   , 25*se_m_perish_song_mvl/mxv
	.byte		PAN   , c_v-17
	.byte		N48   , Cn4 , v100
	.byte	W03
	.byte		VOL   , 29*se_m_perish_song_mvl/mxv
	.byte	W03
@ 001   ----------------------------------------
	.byte		        33*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        40*se_m_perish_song_mvl/mxv
	.byte	W03
@ 002   ----------------------------------------
se_m_perish_song_2_002:
	.byte		VOL   , 45*se_m_perish_song_mvl/mxv
	.byte		MOD   , 8
	.byte	W03
	.byte		VOL   , 51*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte	PEND
@ 003   ----------------------------------------
	.byte		        56*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        62*se_m_perish_song_mvl/mxv
	.byte	W03
@ 004   ----------------------------------------
	.byte		        72*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        81*se_m_perish_song_mvl/mxv
	.byte	W03
@ 005   ----------------------------------------
	.byte		        92*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        100*se_m_perish_song_mvl/mxv
	.byte	W03
@ 006   ----------------------------------------
	.byte	W06
@ 007   ----------------------------------------
	.byte		        82*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        52*se_m_perish_song_mvl/mxv
	.byte	W03
@ 008   ----------------------------------------
	.byte		        25*se_m_perish_song_mvl/mxv
	.byte		MOD   , 0
	.byte		N72   , Bn3 , v100
	.byte	W03
	.byte		VOL   , 29*se_m_perish_song_mvl/mxv
	.byte	W03
@ 009   ----------------------------------------
	.byte		        33*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        40*se_m_perish_song_mvl/mxv
	.byte	W03
@ 010   ----------------------------------------
	.byte	PATT
	 .word	se_m_perish_song_2_002
@ 011   ----------------------------------------
	.byte		VOL   , 56*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        62*se_m_perish_song_mvl/mxv
	.byte	W03
@ 012   ----------------------------------------
	.byte		        72*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        81*se_m_perish_song_mvl/mxv
	.byte	W03
@ 013   ----------------------------------------
	.byte		        92*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        100*se_m_perish_song_mvl/mxv
	.byte	W03
@ 014   ----------------------------------------
	.byte	W06
@ 015   ----------------------------------------
	.byte	W06
@ 016   ----------------------------------------
	.byte	W03
	.byte		        87*se_m_perish_song_mvl/mxv
	.byte	W03
@ 017   ----------------------------------------
	.byte		        75*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        62*se_m_perish_song_mvl/mxv
	.byte	W03
@ 018   ----------------------------------------
	.byte		        48*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        33*se_m_perish_song_mvl/mxv
	.byte	W03
@ 019   ----------------------------------------
	.byte		        25*se_m_perish_song_mvl/mxv
	.byte	W03
	.byte		        10*se_m_perish_song_mvl/mxv
	.byte	W03
@ 020   ----------------------------------------
	.byte	W06
@ 021   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

se_m_perish_song:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_perish_song_pri	@ Priority
	.byte	se_m_perish_song_rev	@ Reverb.

	.word	se_m_perish_song_grp

	.word	se_m_perish_song_1
	.word	se_m_perish_song_2

	.end
