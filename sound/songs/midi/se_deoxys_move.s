	.include "MPlayDef.s"

	.equ	se_deoxys_move_grp, voicegroup129
	.equ	se_deoxys_move_pri, 5
	.equ	se_deoxys_move_rev, reverb_set+50
	.equ	se_deoxys_move_mvl, 127
	.equ	se_deoxys_move_key, 0
	.equ	se_deoxys_move_tbs, 1
	.equ	se_deoxys_move_exg, 1
	.equ	se_deoxys_move_cmp, 1

	.section .rodata
	.global	se_deoxys_move
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_deoxys_move_1:
	.byte	KEYSH , se_deoxys_move_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 262*se_deoxys_move_tbs/2
	.byte		VOICE , 123
	.byte		XCMD  , xIECV , 8
	.byte		        xIECL , 8
	.byte		PAN   , c_v+0
	.byte		VOL   , 40*se_deoxys_move_mvl/mxv
	.byte		BEND  , c_v+2
	.byte		N02   , Cn2 , v120
	.byte	W08
	.byte		N04   , Cn5 
	.byte	W16
	.byte	W24
	.byte	W24
	.byte	W24
@ 002   ----------------------------------------
	.byte	W24
	.byte	W24
	.byte	W24
	.byte	W24
@ 003   ----------------------------------------
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_deoxys_move_2:
	.byte	KEYSH , se_deoxys_move_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 31
	.byte		LFOS  , 44
	.byte		BENDR , 12
	.byte		VOL   , 80*se_deoxys_move_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v-64
	.byte		TIE   , En4 , v032
	.byte	W24
	.byte	W04
	.byte		BEND  , c_v-55
	.byte	W02
	.byte		        c_v-50
	.byte	W02
	.byte		        c_v-39
	.byte	W02
	.byte		        c_v-32
	.byte	W02
	.byte		        c_v-23
	.byte	W02
	.byte		        c_v-18
	.byte	W02
	.byte		        c_v-9
	.byte	W02
	.byte		        c_v-2
	.byte	W02
	.byte		        c_v+16
	.byte	W02
	.byte		        c_v+23
	.byte	W02
	.byte		        c_v+32
	.byte	W02
	.byte		        c_v+39
	.byte	W02
	.byte		        c_v+48
	.byte	W02
	.byte		        c_v+56
	.byte	W02
	.byte		        c_v+63
	.byte	W16
	.byte	W24
@ 001   ----------------------------------------
	.byte	W24
	.byte		VOL   , 70*se_deoxys_move_mvl/mxv
	.byte	W04
	.byte		        66*se_deoxys_move_mvl/mxv
	.byte	W06
	.byte		        60*se_deoxys_move_mvl/mxv
	.byte	W06
	.byte		        55*se_deoxys_move_mvl/mxv
	.byte	W08
	.byte		        49*se_deoxys_move_mvl/mxv
	.byte	W06
	.byte		        44*se_deoxys_move_mvl/mxv
	.byte	W06
	.byte		        40*se_deoxys_move_mvl/mxv
	.byte	W06
	.byte		        35*se_deoxys_move_mvl/mxv
	.byte	W06
	.byte		        30*se_deoxys_move_mvl/mxv
	.byte	W04
	.byte		        24*se_deoxys_move_mvl/mxv
	.byte	W02
	.byte		        20*se_deoxys_move_mvl/mxv
	.byte	W02
	.byte		        15*se_deoxys_move_mvl/mxv
	.byte	W04
	.byte		        10*se_deoxys_move_mvl/mxv
	.byte	W06
	.byte		        0*se_deoxys_move_mvl/mxv
	.byte	W06
	.byte		EOT   
@ 002   ----------------------------------------
	.byte	FINE

@**************** Track 3 (Midi-Chn.3) ****************@

se_deoxys_move_3:
	.byte	KEYSH , se_deoxys_move_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 50
	.byte		VOL   , 69*se_deoxys_move_mvl/mxv
	.byte		PAN   , c_v-64
	.byte	W24
	.byte		        c_v+63
	.byte		N01   , Fs4 , v064
	.byte	W06
	.byte		        Gs4 
	.byte	W03
	.byte		PAN   , c_v-64
	.byte	W07
	.byte		N01   , En5 
	.byte	W05
	.byte		        Fs5 
	.byte	W03
	.byte		PAN   , c_v+63
	.byte	W10
	.byte		N01   , Dn5 
	.byte	W03
	.byte		        Cn5 
	.byte	W07
	.byte		PAN   , c_v-64
	.byte	W04
	.byte		N01   , Fs3 
	.byte	W03
	.byte		        Gs3 
	.byte	W09
	.byte		PAN   , c_v+63
	.byte	W04
	.byte		N01   , Bn4 
	.byte	W04
	.byte		        Dn5 
	.byte	W04
@ 001   ----------------------------------------
	.byte		PAN   , c_v-64
	.byte	W09
	.byte		N01   , Fs5 
	.byte	W09
	.byte		        Cs5 
	.byte	W06
	.byte		PAN   , c_v+63
	.byte	W02
	.byte		N01   , Cs4 , v036
	.byte	W02
	.byte		        As3 
	.byte	W08
	.byte		        Fs3 
	.byte	W03
	.byte		        Gs3 
	.byte	W09
	.byte		PAN   , c_v-64
	.byte	W03
	.byte		N01   , Dn5 , v028
	.byte	W03
	.byte		        Cn5 
	.byte	W06
	.byte		        Fs4 
	.byte	W03
	.byte		        Cn4 
	.byte	W04
	.byte		        Gn3 
	.byte	W05
	.byte		PAN   , c_v+63
	.byte	W06
	.byte		N01   , Gn4 , v020
	.byte	W02
	.byte		        An4 
	.byte	W16
@ 002   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

se_deoxys_move:
	.byte	3	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_deoxys_move_pri	@ Priority
	.byte	se_deoxys_move_rev	@ Reverb.

	.word	se_deoxys_move_grp

	.word	se_deoxys_move_1
	.word	se_deoxys_move_2
	.word	se_deoxys_move_3

	.end
