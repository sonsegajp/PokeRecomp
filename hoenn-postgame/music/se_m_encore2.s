	.include "MPlayDef.s"

	.equ	se_m_encore2_grp, voicegroup111
	.equ	se_m_encore2_pri, 4
	.equ	se_m_encore2_rev, reverb_set+50
	.equ	se_m_encore2_mvl, 100
	.equ	se_m_encore2_key, 0
	.equ	se_m_encore2_tbs, 1
	.equ	se_m_encore2_exg, 1
	.equ	se_m_encore2_cmp, 1

	.section .rodata
	.global	se_m_encore2
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_encore2_1:
	.byte	KEYSH , se_m_encore2_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_encore2_tbs/2
	.byte		VOICE , 60
	.byte		BENDR , 12
	.byte		VOL   , 32*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v+0
	.byte		TIE   , Cn3 , v127
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W01
	.byte		        c_v+5
	.byte	W01
	.byte		VOL   , 44*se_m_encore2_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 59*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		VOL   , 72*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
@ 002   ----------------------------------------
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		        c_v+5
	.byte	W01
	.byte		VOL   , 89*se_m_encore2_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 102*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		VOL   , 112*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
@ 003   ----------------------------------------
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		VOL   , 122*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v+5
	.byte	W02
	.byte		        c_v+2
	.byte	W02
	.byte		VOL   , 127*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		        c_v-2
	.byte	W02
@ 004   ----------------------------------------
se_m_encore2_1_004:
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		        c_v+5
	.byte	W02
	.byte		        c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		        c_v-2
	.byte	W02
	.byte	PEND
@ 005   ----------------------------------------
	.byte	PATT
	 .word	se_m_encore2_1_004
@ 006   ----------------------------------------
	.byte	PATT
	 .word	se_m_encore2_1_004
@ 007   ----------------------------------------
	.byte	PATT
	 .word	se_m_encore2_1_004
@ 008   ----------------------------------------
	.byte	PATT
	 .word	se_m_encore2_1_004
@ 009   ----------------------------------------
	.byte	PATT
	 .word	se_m_encore2_1_004
@ 010   ----------------------------------------
	.byte	PATT
	 .word	se_m_encore2_1_004
@ 011   ----------------------------------------
	.byte		VOL   , 122*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		        c_v+5
	.byte	W01
	.byte		VOL   , 117*se_m_encore2_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 107*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		VOL   , 92*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
@ 012   ----------------------------------------
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		VOL   , 81*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v+5
	.byte	W02
	.byte		VOL   , 67*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 47*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W01
	.byte		VOL   , 28*se_m_encore2_mvl/mxv
	.byte	W01
	.byte		        16*se_m_encore2_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
	.byte		EOT   , Cn3 
@ 013   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

se_m_encore2:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_encore2_pri	@ Priority
	.byte	se_m_encore2_rev	@ Reverb.

	.word	se_m_encore2_grp

	.word	se_m_encore2_1

	.end
