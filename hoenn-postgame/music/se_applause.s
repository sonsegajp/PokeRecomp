	.include "MPlayDef.s"

	.equ	se_applause_grp, voicegroup111
	.equ	se_applause_pri, 5
	.equ	se_applause_rev, reverb_set+50
	.equ	se_applause_mvl, 100
	.equ	se_applause_key, 0
	.equ	se_applause_tbs, 1
	.equ	se_applause_exg, 1
	.equ	se_applause_cmp, 1

	.section .rodata
	.global	se_applause
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_applause_1:
	.byte	KEYSH , se_applause_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_applause_tbs/2
	.byte		VOICE , 60
	.byte		BENDR , 12
	.byte		VOL   , 32*se_applause_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v+0
	.byte		TIE   , Bn2 , v127
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W01
	.byte		        c_v+5
	.byte	W01
	.byte		VOL   , 44*se_applause_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 59*se_applause_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		VOL   , 72*se_applause_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
@ 002   ----------------------------------------
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		        c_v+5
	.byte	W01
	.byte		VOL   , 89*se_applause_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 102*se_applause_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		VOL   , 112*se_applause_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
@ 003   ----------------------------------------
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		VOL   , 122*se_applause_mvl/mxv
	.byte		PAN   , c_v+5
	.byte	W02
	.byte		        c_v+2
	.byte	W02
	.byte		VOL   , 127*se_applause_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		        c_v-2
	.byte	W02
@ 004   ----------------------------------------
se_applause_1_004:
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
	 .word	se_applause_1_004
@ 006   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 007   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 008   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 009   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 010   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 011   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 012   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 013   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 014   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 015   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 016   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 017   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 018   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 019   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 020   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 021   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 022   ----------------------------------------
	.byte	PATT
	 .word	se_applause_1_004
@ 023   ----------------------------------------
	.byte		VOL   , 122*se_applause_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		        c_v+5
	.byte	W01
	.byte		VOL   , 117*se_applause_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 107*se_applause_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		VOL   , 92*se_applause_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
@ 024   ----------------------------------------
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		VOL   , 81*se_applause_mvl/mxv
	.byte		PAN   , c_v+5
	.byte	W02
	.byte		VOL   , 67*se_applause_mvl/mxv
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 47*se_applause_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W01
	.byte		VOL   , 28*se_applause_mvl/mxv
	.byte	W01
	.byte		        16*se_applause_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
	.byte		EOT   , Bn2 
@ 025   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

se_applause:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_applause_pri	@ Priority
	.byte	se_applause_rev	@ Reverb.

	.word	se_applause_grp

	.word	se_applause_1

	.end
