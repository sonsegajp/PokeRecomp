	.include "MPlayDef.s"

	.equ	se_m_self_destruct_grp, voicegroup111
	.equ	se_m_self_destruct_pri, 4
	.equ	se_m_self_destruct_rev, reverb_set+50
	.equ	se_m_self_destruct_mvl, 115
	.equ	se_m_self_destruct_key, 0
	.equ	se_m_self_destruct_tbs, 1
	.equ	se_m_self_destruct_exg, 1
	.equ	se_m_self_destruct_cmp, 1

	.section .rodata
	.global	se_m_self_destruct
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_self_destruct_1:
	.byte	KEYSH , se_m_self_destruct_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_self_destruct_tbs/2
	.byte		VOICE , 41
	.byte		VOL   , 127*se_m_self_destruct_mvl/mxv
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v+0
	.byte		N01   , Gn3 , v127
	.byte	W01
	.byte		        Cn3 
	.byte	W01
	.byte		N24   , An3 
	.byte	W06
	.byte		PAN   , c_v+6
	.byte	W04
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 113*se_m_self_destruct_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-8
	.byte		VOL   , 98*se_m_self_destruct_mvl/mxv
	.byte	W03
	.byte		        87*se_m_self_destruct_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 74*se_m_self_destruct_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v-2
	.byte	W01
	.byte		PAN   , c_v+7
	.byte		VOL   , 50*se_m_self_destruct_mvl/mxv
	.byte	W02
	.byte		BEND  , c_v-8
	.byte	W01
	.byte		VOL   , 15*se_m_self_destruct_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v-15
	.byte	W22
	.byte	FINE

@******************************************************@
	.align	2

se_m_self_destruct:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_self_destruct_pri	@ Priority
	.byte	se_m_self_destruct_rev	@ Reverb.

	.word	se_m_self_destruct_grp

	.word	se_m_self_destruct_1

	.end
