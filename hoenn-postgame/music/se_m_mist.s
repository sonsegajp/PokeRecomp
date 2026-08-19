	.include "MPlayDef.s"

	.equ	se_m_mist_grp, voicegroup111
	.equ	se_m_mist_pri, 4
	.equ	se_m_mist_rev, reverb_set+50
	.equ	se_m_mist_mvl, 110
	.equ	se_m_mist_key, 0
	.equ	se_m_mist_tbs, 1
	.equ	se_m_mist_exg, 1
	.equ	se_m_mist_cmp, 1

	.section .rodata
	.global	se_m_mist
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_mist_1:
	.byte	KEYSH , se_m_mist_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_mist_tbs/2
	.byte		VOICE , 36
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 9*se_m_mist_mvl/mxv
	.byte		BEND  , c_v+1
	.byte		N18   , Cn5 , v052
	.byte	W01
	.byte		VOL   , 24*se_m_mist_mvl/mxv
	.byte		PAN   , c_v+16
	.byte		BEND  , c_v-8
	.byte	W01
	.byte		VOL   , 48*se_m_mist_mvl/mxv
	.byte		PAN   , c_v-14
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		VOL   , 81*se_m_mist_mvl/mxv
	.byte		PAN   , c_v+16
	.byte		BEND  , c_v-8
	.byte	W01
	.byte		VOL   , 108*se_m_mist_mvl/mxv
	.byte		PAN   , c_v-14
	.byte		BEND  , c_v+0
	.byte	W02
	.byte		PAN   , c_v+0
	.byte		VOL   , 127*se_m_mist_mvl/mxv
	.byte		BEND  , c_v+8
	.byte	W01
	.byte		PAN   , c_v+16
	.byte		BEND  , c_v+16
	.byte	W01
	.byte		PAN   , c_v-14
	.byte	W01
	.byte		VOL   , 114*se_m_mist_mvl/mxv
	.byte		PAN   , c_v+16
	.byte		BEND  , c_v+10
	.byte	W01
	.byte		VOL   , 99*se_m_mist_mvl/mxv
	.byte		PAN   , c_v-14
	.byte		BEND  , c_v+4
	.byte	W02
	.byte		VOL   , 80*se_m_mist_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v-7
	.byte	W01
	.byte		VOL   , 59*se_m_mist_mvl/mxv
	.byte		PAN   , c_v+16
	.byte		BEND  , c_v-22
	.byte	W01
	.byte		PAN   , c_v-14
	.byte		VOL   , 44*se_m_mist_mvl/mxv
	.byte		BEND  , c_v-28
	.byte	W01
	.byte		VOL   , 20*se_m_mist_mvl/mxv
	.byte		BEND  , c_v-32
	.byte	W01
	.byte		VOL   , 5*se_m_mist_mvl/mxv
	.byte		BEND  , c_v-38
	.byte	W08
	.byte	FINE

@******************************************************@
	.align	2

se_m_mist:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_mist_pri	@ Priority
	.byte	se_m_mist_rev	@ Reverb.

	.word	se_m_mist_grp

	.word	se_m_mist_1

	.end
