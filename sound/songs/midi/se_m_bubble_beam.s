	.include "MPlayDef.s"

	.equ	se_m_bubble_beam_grp, voicegroup128
	.equ	se_m_bubble_beam_pri, 4
	.equ	se_m_bubble_beam_rev, reverb_set+50
	.equ	se_m_bubble_beam_mvl, 127
	.equ	se_m_bubble_beam_key, 0
	.equ	se_m_bubble_beam_tbs, 1
	.equ	se_m_bubble_beam_exg, 1
	.equ	se_m_bubble_beam_cmp, 1

	.section .rodata
	.global	se_m_bubble_beam
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_bubble_beam_1:
	.byte	KEYSH , se_m_bubble_beam_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_bubble_beam_tbs/2
	.byte		VOICE , 9
	.byte		BENDR , 2
	.byte		PAN   , c_v+0
	.byte		VOL   , 24*se_m_bubble_beam_mvl/mxv
	.byte		BEND  , c_v-17
	.byte		N78   , Bn4 , v080
	.byte	W01
	.byte		VOL   , 32*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		VOL   , 38*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v-7
	.byte	W01
	.byte		VOL   , 48*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		VOL   , 66*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v-13
	.byte	W02
@ 002   ----------------------------------------
	.byte		VOL   , 78*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		VOL   , 94*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v-6
	.byte	W01
	.byte		VOL   , 110*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v-7
	.byte	W02
@ 003   ----------------------------------------
se_m_bubble_beam_1_003:
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		        c_v-13
	.byte	W01
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v-6
	.byte	W01
	.byte		        c_v+0
	.byte	W02
	.byte	PEND
@ 004   ----------------------------------------
se_m_bubble_beam_1_004:
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		        c_v-7
	.byte	W01
	.byte		        c_v+9
	.byte	W01
	.byte		        c_v-13
	.byte	W01
	.byte		        c_v+6
	.byte	W02
	.byte	PEND
@ 005   ----------------------------------------
	.byte		        c_v-6
	.byte	W01
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v-7
	.byte	W01
	.byte		        c_v+9
	.byte	W02
@ 006   ----------------------------------------
	.byte		        c_v-13
	.byte	W01
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v-6
	.byte	W01
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+6
	.byte	W02
@ 007   ----------------------------------------
	.byte		        c_v-7
	.byte	W01
	.byte		        c_v+9
	.byte	W01
	.byte		        c_v-13
	.byte	W01
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v-6
	.byte	W02
@ 008   ----------------------------------------
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v-7
	.byte	W01
	.byte		        c_v+9
	.byte	W01
	.byte		        c_v-13
	.byte	W02
@ 009   ----------------------------------------
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v-6
	.byte	W01
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v-7
	.byte	W02
@ 010   ----------------------------------------
	.byte	PATT
	 .word	se_m_bubble_beam_1_003
@ 011   ----------------------------------------
	.byte	PATT
	 .word	se_m_bubble_beam_1_004
@ 012   ----------------------------------------
	.byte		PAN   , c_v-6
	.byte	W01
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+6
	.byte	W01
	.byte		VOL   , 103*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v-7
	.byte	W01
	.byte		VOL   , 91*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v+9
	.byte	W02
@ 013   ----------------------------------------
	.byte		VOL   , 72*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v-13
	.byte	W01
	.byte		VOL   , 58*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		VOL   , 38*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v-6
	.byte	W01
	.byte		VOL   , 15*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 7*se_m_bubble_beam_mvl/mxv
	.byte		PAN   , c_v+6
	.byte	W02
@ 014   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

se_m_bubble_beam:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_bubble_beam_pri	@ Priority
	.byte	se_m_bubble_beam_rev	@ Reverb.

	.word	se_m_bubble_beam_grp

	.word	se_m_bubble_beam_1

	.end
