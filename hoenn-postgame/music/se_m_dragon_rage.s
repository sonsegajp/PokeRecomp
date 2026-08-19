	.include "MPlayDef.s"

	.equ	se_m_dragon_rage_grp, voicegroup111
	.equ	se_m_dragon_rage_pri, 4
	.equ	se_m_dragon_rage_rev, reverb_set+50
	.equ	se_m_dragon_rage_mvl, 120
	.equ	se_m_dragon_rage_key, 0
	.equ	se_m_dragon_rage_tbs, 1
	.equ	se_m_dragon_rage_exg, 1
	.equ	se_m_dragon_rage_cmp, 1

	.section .rodata
	.global	se_m_dragon_rage
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_dragon_rage_1:
	.byte	KEYSH , se_m_dragon_rage_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_dragon_rage_tbs/2
	.byte		VOICE , 29
	.byte		BENDR , 12
	.byte		PAN   , c_v+5
	.byte		VOL   , 23*se_m_dragon_rage_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N10   , An2 , v127
	.byte	W02
	.byte		VOL   , 50*se_m_dragon_rage_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 94*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-5
	.byte	W01
	.byte		VOL   , 127*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 23*se_m_dragon_rage_mvl/mxv
	.byte		BEND  , c_v+10
	.byte		N10   , Bn2 
	.byte	W02
	.byte		PAN   , c_v+5
	.byte	W01
	.byte		VOL   , 50*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+0
	.byte		VOL   , 94*se_m_dragon_rage_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-5
	.byte		VOL   , 127*se_m_dragon_rage_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-2
	.byte		VOL   , 23*se_m_dragon_rage_mvl/mxv
	.byte		BEND  , c_v+18
	.byte		N10   , Cs3 
	.byte	W03
	.byte		PAN   , c_v+5
	.byte		VOL   , 50*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		        94*se_m_dragon_rage_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 127*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-5
	.byte	W02
	.byte		VOL   , 22*se_m_dragon_rage_mvl/mxv
	.byte		BEND  , c_v+29
	.byte		N10   , Dn3 
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 50*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+5
	.byte	W01
	.byte		VOL   , 94*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+0
	.byte		VOL   , 127*se_m_dragon_rage_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-5
	.byte	W01
	.byte		VOL   , 23*se_m_dragon_rage_mvl/mxv
	.byte		BEND  , c_v+37
	.byte		N10   , En3 
	.byte	W02
	.byte		PAN   , c_v-2
	.byte		VOL   , 49*se_m_dragon_rage_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+5
	.byte		VOL   , 108*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		        127*se_m_dragon_rage_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W03
	.byte		        c_v-5
	.byte		VOL   , 22*se_m_dragon_rage_mvl/mxv
	.byte		BEND  , c_v+42
	.byte		N20   , Fs3 
	.byte	W02
	.byte		VOL   , 50*se_m_dragon_rage_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 94*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+5
	.byte	W01
	.byte		VOL   , 127*se_m_dragon_rage_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 105*se_m_dragon_rage_mvl/mxv
	.byte		BEND  , c_v+50
	.byte	W02
	.byte		PAN   , c_v-5
	.byte	W01
	.byte		VOL   , 79*se_m_dragon_rage_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+56
	.byte	W01
	.byte		PAN   , c_v-2
	.byte		VOL   , 57*se_m_dragon_rage_mvl/mxv
	.byte	W03
	.byte		        36*se_m_dragon_rage_mvl/mxv
	.byte		BEND  , c_v+63
	.byte	W01
	.byte		VOL   , 16*se_m_dragon_rage_mvl/mxv
	.byte	W23
@ 002   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

se_m_dragon_rage:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_dragon_rage_pri	@ Priority
	.byte	se_m_dragon_rage_rev	@ Reverb.

	.word	se_m_dragon_rage_grp

	.word	se_m_dragon_rage_1

	.end
