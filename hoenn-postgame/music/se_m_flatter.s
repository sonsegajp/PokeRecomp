	.include "MPlayDef.s"

	.equ	se_m_flatter_grp, voicegroup111
	.equ	se_m_flatter_pri, 4
	.equ	se_m_flatter_rev, reverb_set+50
	.equ	se_m_flatter_mvl, 100
	.equ	se_m_flatter_key, 0
	.equ	se_m_flatter_tbs, 1
	.equ	se_m_flatter_exg, 1
	.equ	se_m_flatter_cmp, 1

	.section .rodata
	.global	se_m_flatter
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_flatter_1:
	.byte	KEYSH , se_m_flatter_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_flatter_tbs/2
	.byte		VOICE , 41
	.byte		VOL   , 127*se_m_flatter_mvl/mxv
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v+0
	.byte		N01   , Dn5 , v127
	.byte	W01
	.byte		        Gn4 
	.byte	W01
	.byte		N09   , En5 
	.byte	W02
	.byte		PAN   , c_v+6
	.byte	W02
	.byte		VOL   , 117*se_m_flatter_mvl/mxv
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		VOL   , 106*se_m_flatter_mvl/mxv
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		VOL   , 91*se_m_flatter_mvl/mxv
	.byte		PAN   , c_v-8
	.byte		BEND  , c_v-4
	.byte	W01
	.byte		VOL   , 59*se_m_flatter_mvl/mxv
	.byte		PAN   , c_v+11
	.byte		BEND  , c_v-8
	.byte	W01
	.byte		VOL   , 28*se_m_flatter_mvl/mxv
	.byte		PAN   , c_v-12
	.byte		BEND  , c_v-15
	.byte	W02
	.byte	FINE

@******************************************************@
	.align	2

se_m_flatter:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_flatter_pri	@ Priority
	.byte	se_m_flatter_rev	@ Reverb.

	.word	se_m_flatter_grp

	.word	se_m_flatter_1

	.end
