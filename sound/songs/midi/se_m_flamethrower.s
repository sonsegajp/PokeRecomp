	.include "MPlayDef.s"

	.equ	se_m_flamethrower_grp, voicegroup128
	.equ	se_m_flamethrower_pri, 4
	.equ	se_m_flamethrower_rev, reverb_set+50
	.equ	se_m_flamethrower_mvl, 127
	.equ	se_m_flamethrower_key, 0
	.equ	se_m_flamethrower_tbs, 1
	.equ	se_m_flamethrower_exg, 1
	.equ	se_m_flamethrower_cmp, 1

	.section .rodata
	.global	se_m_flamethrower
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_flamethrower_1:
	.byte	KEYSH , se_m_flamethrower_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_flamethrower_tbs/2
	.byte		VOICE , 29
	.byte		BENDR , 12
	.byte		VOL   , 110*se_m_flamethrower_mvl/mxv
	.byte		PAN   , c_v+4
	.byte		BEND  , c_v+50
	.byte		TIE   , Cn3 , v127
	.byte	W01
	.byte		BEND  , c_v+32
	.byte	W02
	.byte		PAN   , c_v-5
	.byte		BEND  , c_v+16
	.byte	W02
	.byte		        c_v-8
	.byte	W02
	.byte		PAN   , c_v+8
	.byte		BEND  , c_v-15
	.byte	W02
	.byte		        c_v-10
	.byte	W01
	.byte		PAN   , c_v-9
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W02
	.byte		PAN   , c_v+4
	.byte		BEND  , c_v+2
	.byte	W04
	.byte		PAN   , c_v-5
	.byte		BEND  , c_v+10
	.byte	W03
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v+16
	.byte	W03
@ 002   ----------------------------------------
	.byte		PAN   , c_v+4
	.byte	W01
	.byte		BEND  , c_v+21
	.byte	W02
	.byte		PAN   , c_v-5
	.byte	W04
	.byte		        c_v+8
	.byte	W03
	.byte		        c_v-9
	.byte	W04
	.byte		        c_v+4
	.byte	W04
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v+0
	.byte	W03
@ 003   ----------------------------------------
se_m_flamethrower_1_003:
	.byte		PAN   , c_v+4
	.byte	W03
	.byte		        c_v-5
	.byte	W04
	.byte		        c_v+8
	.byte	W03
	.byte		        c_v-9
	.byte	W04
	.byte		        c_v+4
	.byte	W04
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v+0
	.byte	W03
	.byte	PEND
@ 004   ----------------------------------------
	.byte	PATT
	 .word	se_m_flamethrower_1_003
@ 005   ----------------------------------------
	.byte	PATT
	 .word	se_m_flamethrower_1_003
@ 006   ----------------------------------------
	.byte	PATT
	 .word	se_m_flamethrower_1_003
@ 007   ----------------------------------------
	.byte		PAN   , c_v+4
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		VOL   , 103*se_m_flamethrower_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+8
	.byte	W03
	.byte		VOL   , 96*se_m_flamethrower_mvl/mxv
	.byte		PAN   , c_v-9
	.byte	W04
	.byte		        c_v+4
	.byte	W02
	.byte		VOL   , 88*se_m_flamethrower_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-5
	.byte	W03
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 73*se_m_flamethrower_mvl/mxv
	.byte	W02
@ 008   ----------------------------------------
	.byte		PAN   , c_v+4
	.byte	W03
	.byte		VOL   , 62*se_m_flamethrower_mvl/mxv
	.byte		PAN   , c_v-5
	.byte	W04
	.byte		        c_v+8
	.byte	W01
	.byte		VOL   , 47*se_m_flamethrower_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-9
	.byte	W03
	.byte		VOL   , 27*se_m_flamethrower_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+4
	.byte	W04
	.byte		VOL   , 10*se_m_flamethrower_mvl/mxv
	.byte		PAN   , c_v-5
	.byte	W03
	.byte		        c_v+0
	.byte	W03
	.byte		EOT   , Cn3 
@ 009   ----------------------------------------
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_m_flamethrower_2:
	.byte	KEYSH , se_m_flamethrower_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 27
	.byte		PAN   , c_v+0
	.byte		VOL   , 110*se_m_flamethrower_mvl/mxv
	.byte		N24   , Fn2 , v060
	.byte	W10
	.byte		VOL   , 99*se_m_flamethrower_mvl/mxv
	.byte	W05
	.byte		        82*se_m_flamethrower_mvl/mxv
	.byte	W04
	.byte		        54*se_m_flamethrower_mvl/mxv
	.byte	W02
	.byte		        25*se_m_flamethrower_mvl/mxv
	.byte	W03
@ 001   ----------------------------------------
	.byte	W24
@ 002   ----------------------------------------
	.byte	W24
@ 003   ----------------------------------------
	.byte	W24
@ 004   ----------------------------------------
	.byte	W24
@ 005   ----------------------------------------
	.byte	W24
@ 006   ----------------------------------------
	.byte	W24
@ 007   ----------------------------------------
	.byte	W24
@ 008   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

se_m_flamethrower:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_flamethrower_pri	@ Priority
	.byte	se_m_flamethrower_rev	@ Reverb.

	.word	se_m_flamethrower_grp

	.word	se_m_flamethrower_1
	.word	se_m_flamethrower_2

	.end
