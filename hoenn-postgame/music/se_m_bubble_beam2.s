	.include "MPlayDef.s"

	.equ	se_m_bubble_beam2_grp, voicegroup111
	.equ	se_m_bubble_beam2_pri, 4
	.equ	se_m_bubble_beam2_rev, reverb_set+50
	.equ	se_m_bubble_beam2_mvl, 120
	.equ	se_m_bubble_beam2_key, 0
	.equ	se_m_bubble_beam2_tbs, 1
	.equ	se_m_bubble_beam2_exg, 1
	.equ	se_m_bubble_beam2_cmp, 1

	.section .rodata
	.global	se_m_bubble_beam2
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_bubble_beam2_1:
	.byte	KEYSH , se_m_bubble_beam2_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_bubble_beam2_tbs/2
	.byte		VOICE , 9
	.byte		BENDR , 2
	.byte		PAN   , c_v+0
	.byte		VOL   , 28*se_m_bubble_beam2_mvl/mxv
	.byte		BEND  , c_v-23
	.byte		N09   , Dn5 , v127
	.byte	W01
	.byte		VOL   , 36*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		BEND  , c_v-20
	.byte	W01
	.byte		VOL   , 45*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-7
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		VOL   , 56*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		VOL   , 77*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-13
	.byte		BEND  , c_v-18
	.byte	W02
	.byte		VOL   , 90*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		BEND  , c_v-21
	.byte	W01
	.byte		VOL   , 108*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-6
	.byte		BEND  , c_v-23
	.byte	W01
	.byte		VOL   , 127*se_m_bubble_beam2_mvl/mxv
	.byte		BEND  , c_v-24
	.byte	W02
	.byte		VOL   , 28*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v-23
	.byte		N09   , Dn5 , v100
	.byte	W02
	.byte		VOL   , 36*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		BEND  , c_v-20
	.byte	W01
	.byte		VOL   , 45*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-7
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		VOL   , 56*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		VOL   , 77*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-13
	.byte		BEND  , c_v-18
	.byte	W01
	.byte		VOL   , 90*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		BEND  , c_v-21
	.byte	W02
	.byte		VOL   , 108*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-6
	.byte		BEND  , c_v-23
	.byte	W01
	.byte		VOL   , 127*se_m_bubble_beam2_mvl/mxv
	.byte		BEND  , c_v-24
	.byte	W02
	.byte		VOL   , 28*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v-23
	.byte		N09   , Dn5 , v080
	.byte	W01
	.byte		VOL   , 36*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		BEND  , c_v-20
	.byte	W02
	.byte		VOL   , 45*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-7
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		VOL   , 56*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		VOL   , 77*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-13
	.byte		BEND  , c_v-18
	.byte	W01
	.byte		VOL   , 90*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		BEND  , c_v-21
	.byte	W01
	.byte		VOL   , 108*se_m_bubble_beam2_mvl/mxv
	.byte		PAN   , c_v-6
	.byte		BEND  , c_v-23
	.byte	W02
	.byte		VOL   , 127*se_m_bubble_beam2_mvl/mxv
	.byte		BEND  , c_v-24
	.byte	W06
	.byte	FINE

@******************************************************@
	.align	2

se_m_bubble_beam2:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_bubble_beam2_pri	@ Priority
	.byte	se_m_bubble_beam2_rev	@ Reverb.

	.word	se_m_bubble_beam2_grp

	.word	se_m_bubble_beam2_1

	.end
