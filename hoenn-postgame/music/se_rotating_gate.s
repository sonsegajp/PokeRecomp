	.include "MPlayDef.s"

	.equ	se_rotating_gate_grp, voicegroup111
	.equ	se_rotating_gate_pri, 4
	.equ	se_rotating_gate_rev, reverb_set+50
	.equ	se_rotating_gate_mvl, 90
	.equ	se_rotating_gate_key, 0
	.equ	se_rotating_gate_tbs, 1
	.equ	se_rotating_gate_exg, 1
	.equ	se_rotating_gate_cmp, 1

	.section .rodata
	.global	se_rotating_gate
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_rotating_gate_1:
	.byte	KEYSH , se_rotating_gate_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 220*se_rotating_gate_tbs/2
	.byte		VOICE , 9
	.byte		VOL   , 127*se_rotating_gate_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N03   , Gn4 , v112
	.byte	W03
	.byte		        Dn4 
	.byte	W03
	.byte		        Gn4 
	.byte	W03
	.byte		N21   , Gn5 
	.byte	W03
	.byte	W03
	.byte		VOL   , 115*se_rotating_gate_mvl/mxv
	.byte		BEND  , c_v+6
	.byte	W03
	.byte		VOL   , 102*se_rotating_gate_mvl/mxv
	.byte		BEND  , c_v+20
	.byte	W03
	.byte		VOL   , 65*se_rotating_gate_mvl/mxv
	.byte		BEND  , c_v+37
	.byte	W03
	.byte		VOL   , 34*se_rotating_gate_mvl/mxv
	.byte		BEND  , c_v+63
	.byte	W03
	.byte		VOL   , 10*se_rotating_gate_mvl/mxv
	.byte	W03
	.byte	FINE

@******************************************************@
	.align	2

se_rotating_gate:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_rotating_gate_pri	@ Priority
	.byte	se_rotating_gate_rev	@ Reverb.

	.word	se_rotating_gate_grp

	.word	se_rotating_gate_1

	.end
