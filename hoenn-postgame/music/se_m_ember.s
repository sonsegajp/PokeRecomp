	.include "MPlayDef.s"

	.equ	se_m_ember_grp, voicegroup111
	.equ	se_m_ember_pri, 4
	.equ	se_m_ember_rev, reverb_set+50
	.equ	se_m_ember_mvl, 110
	.equ	se_m_ember_key, 0
	.equ	se_m_ember_tbs, 1
	.equ	se_m_ember_exg, 1
	.equ	se_m_ember_cmp, 1

	.section .rodata
	.global	se_m_ember
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_ember_1:
	.byte	KEYSH , se_m_ember_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_ember_tbs/2
	.byte		VOICE , 29
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 71*se_m_ember_mvl/mxv
	.byte		BEND  , c_v+8
	.byte		N10   , Gn3 , v127
	.byte	W01
	.byte		VOL   , 97*se_m_ember_mvl/mxv
	.byte	W01
	.byte		        112*se_m_ember_mvl/mxv
	.byte		BEND  , c_v+16
	.byte	W01
	.byte		VOL   , 127*se_m_ember_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+7
	.byte	W02
	.byte		        c_v-12
	.byte	W01
	.byte		VOL   , 103*se_m_ember_mvl/mxv
	.byte	W01
	.byte		        74*se_m_ember_mvl/mxv
	.byte		BEND  , c_v-42
	.byte	W01
	.byte		VOL   , 40*se_m_ember_mvl/mxv
	.byte	W15
	.byte	FINE

@******************************************************@
	.align	2

se_m_ember:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_ember_pri	@ Priority
	.byte	se_m_ember_rev	@ Reverb.

	.word	se_m_ember_grp

	.word	se_m_ember_1

	.end
