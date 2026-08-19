	.include "MPlayDef.s"

	.equ	mus_heal_unused_grp, voicegroup140
	.equ	mus_heal_unused_pri, 0
	.equ	mus_heal_unused_rev, reverb_set+50
	.equ	mus_heal_unused_mvl, 90
	.equ	mus_heal_unused_key, 0
	.equ	mus_heal_unused_tbs, 1
	.equ	mus_heal_unused_exg, 1
	.equ	mus_heal_unused_cmp, 1

	.section .rodata
	.global	mus_heal_unused
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

mus_heal_unused_1:
	.byte		VOL   , 127*mus_heal_unused_mvl/mxv
	.byte	KEYSH , mus_heal_unused_key+0
@ 000   ----------------------------------------
	.byte	TEMPO , 146*mus_heal_unused_tbs/2
	.byte		VOICE , 2
	.byte		N24   , Bn4 , v064
	.byte	W24
@ 001   ----------------------------------------
	.byte		N24   
	.byte	W24
@ 002   ----------------------------------------
	.byte		N12   
	.byte	W12
	.byte		        Gs4 
	.byte	W12
@ 003   ----------------------------------------
	.byte		N48   , En5 
	.byte	W24
@ 004   ----------------------------------------
	.byte	W24
@ 005   ----------------------------------------
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

mus_heal_unused_2:
	.byte		VOL   , 127*mus_heal_unused_mvl/mxv
	.byte	KEYSH , mus_heal_unused_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 3
	.byte		N12   , En2 , v092
	.byte	W24
@ 001   ----------------------------------------
	.byte		N12   
	.byte	W24
@ 002   ----------------------------------------
	.byte		N12   
	.byte	W12
	.byte		        Gs2 
	.byte	W12
@ 003   ----------------------------------------
	.byte		N36   , En2 
	.byte	W24
@ 004   ----------------------------------------
	.byte	W24
@ 005   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

mus_heal_unused:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	mus_heal_unused_pri	@ Priority
	.byte	mus_heal_unused_rev	@ Reverb.

	.word	mus_heal_unused_grp

	.word	mus_heal_unused_1
	.word	mus_heal_unused_2

	.end
