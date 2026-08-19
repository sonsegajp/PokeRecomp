	.include "MPlayDef.s"

	.equ	se_card_open_grp, voicegroup129
	.equ	se_card_open_pri, 5
	.equ	se_card_open_rev, reverb_set+50
	.equ	se_card_open_mvl, 127
	.equ	se_card_open_key, 0
	.equ	se_card_open_tbs, 1
	.equ	se_card_open_exg, 1
	.equ	se_card_open_cmp, 1

	.section .rodata
	.global	se_card_open
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_card_open_1:
	.byte	KEYSH , se_card_open_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 174*se_card_open_tbs/2
	.byte		VOICE , 46
	.byte		XCMD  , xIECV , 8
	.byte		        xIECL , 8
	.byte		VOL   , 112*se_card_open_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 112*se_card_open_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v+0
	.byte		N03   , An5 , v112
	.byte	W03
	.byte		VOL   , 64*se_card_open_mvl/mxv
	.byte		N03   , Bn5 
	.byte	W03
	.byte		VOL   , 112*se_card_open_mvl/mxv
	.byte		PAN   , c_v-48
	.byte		N06   , En6 
	.byte	W03
	.byte		VOL   , 64*se_card_open_mvl/mxv
	.byte	W03
	.byte		        112*se_card_open_mvl/mxv
	.byte		PAN   , c_v+48
	.byte		N06   , En6 , v040
	.byte	W03
	.byte		VOL   , 64*se_card_open_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-48
	.byte		VOL   , 97*se_card_open_mvl/mxv
	.byte		N06   , En6 , v032
	.byte	W03
	.byte		VOL   , 64*se_card_open_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+48
	.byte		VOL   , 96*se_card_open_mvl/mxv
	.byte		N06   , En6 , v024
	.byte	W03
	.byte		VOL   , 64*se_card_open_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-48
	.byte		VOL   , 80*se_card_open_mvl/mxv
	.byte		N06   , En6 , v016
	.byte	W03
	.byte		VOL   , 64*se_card_open_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+48
	.byte		VOL   , 80*se_card_open_mvl/mxv
	.byte		N06   , En6 , v012
	.byte	W03
	.byte		VOL   , 48*se_card_open_mvl/mxv
	.byte	W03
	.byte		N06   , En6 , v008
	.byte	W03
	.byte		VOL   , 2*se_card_open_mvl/mxv
	.byte	W03
	.byte	W24
	.byte	W24
@ 002   ----------------------------------------
	.byte	W24
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_card_open_2:
	.byte	KEYSH , se_card_open_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 124
	.byte		PAN   , c_v+0
	.byte		VOL   , 64*se_card_open_mvl/mxv
	.byte		N03   , Cn5 , v092
	.byte	W06
	.byte		        Cn5 , v032
	.byte	W18
	.byte	W24
	.byte	W24
	.byte	W24
@ 001   ----------------------------------------
	.byte	W24
	.byte	FINE

@******************************************************@
	.align	2

se_card_open:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_card_open_pri	@ Priority
	.byte	se_card_open_rev	@ Reverb.

	.word	se_card_open_grp

	.word	se_card_open_1
	.word	se_card_open_2

	.end
