	.include "MPlayDef.s"

	.equ	se_m_twister_grp, voicegroup111
	.equ	se_m_twister_pri, 4
	.equ	se_m_twister_rev, reverb_set+50
	.equ	se_m_twister_mvl, 115
	.equ	se_m_twister_key, 0
	.equ	se_m_twister_tbs, 1
	.equ	se_m_twister_exg, 1
	.equ	se_m_twister_cmp, 1

	.section .rodata
	.global	se_m_twister
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_twister_1:
	.byte	KEYSH , se_m_twister_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_twister_tbs/2
	.byte		VOICE , 22
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 74*se_m_twister_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		TIE   , Dn2 , v127
	.byte	W03
	.byte		VOL   , 92*se_m_twister_mvl/mxv
	.byte		BEND  , c_v+22
	.byte	W03
	.byte		VOL   , 98*se_m_twister_mvl/mxv
	.byte		PAN   , c_v-4
	.byte	W03
	.byte		VOL   , 105*se_m_twister_mvl/mxv
	.byte	W02
	.byte		BEND  , c_v+8
	.byte	W01
	.byte		VOL   , 116*se_m_twister_mvl/mxv
	.byte		PAN   , c_v-8
	.byte	W03
	.byte		VOL   , 127*se_m_twister_mvl/mxv
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W01
	.byte		PAN   , c_v-12
	.byte	W06
	.byte		        c_v-17
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		        c_v-7
	.byte	W03
	.byte		PAN   , c_v-9
	.byte	W06
	.byte		        c_v-3
	.byte	W01
	.byte		BEND  , c_v-12
	.byte	W02
	.byte		PAN   , c_v+6
	.byte	W03
	.byte		        c_v+10
	.byte	W06
	.byte		        c_v+16
	.byte	W01
	.byte		BEND  , c_v-20
	.byte	W05
	.byte		PAN   , c_v+13
	.byte	W06
	.byte		        c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W01
	.byte		BEND  , c_v-13
	.byte	W02
	.byte		PAN   , c_v-8
	.byte	W03
	.byte		        c_v-11
	.byte	W03
	.byte		        c_v-17
	.byte	W01
	.byte		BEND  , c_v-6
	.byte	W05
	.byte		PAN   , c_v-9
	.byte	W06
	.byte		        c_v-3
	.byte	W03
	.byte		        c_v+6
	.byte	W03
	.byte		        c_v+10
	.byte	W01
	.byte		BEND  , c_v+6
	.byte	W05
@ 002   ----------------------------------------
	.byte		PAN   , c_v+16
	.byte	W06
	.byte		        c_v+13
	.byte	W06
	.byte		        c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte		BEND  , c_v+11
	.byte	W03
	.byte		PAN   , c_v-8
	.byte	W03
	.byte		        c_v-11
	.byte		VOL   , 114*se_m_twister_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-17
	.byte	W01
	.byte		BEND  , c_v+20
	.byte	W05
	.byte		PAN   , c_v-9
	.byte	W01
	.byte		VOL   , 108*se_m_twister_mvl/mxv
	.byte	W05
	.byte		PAN   , c_v-3
	.byte	W01
	.byte		BEND  , c_v+12
	.byte	W02
	.byte		PAN   , c_v+6
	.byte		VOL   , 98*se_m_twister_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+10
	.byte	W02
	.byte		BEND  , c_v+7
	.byte	W02
	.byte		VOL   , 88*se_m_twister_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+16
	.byte	W04
	.byte		VOL   , 69*se_m_twister_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W02
	.byte		PAN   , c_v+13
	.byte	W04
	.byte		VOL   , 50*se_m_twister_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+8
	.byte	W01
	.byte		BEND  , c_v-7
	.byte	W01
	.byte		VOL   , 37*se_m_twister_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W03
	.byte		        c_v-8
	.byte	W01
	.byte		VOL   , 20*se_m_twister_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-11
	.byte	W03
	.byte		EOT   
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_m_twister_2:
	.byte	KEYSH , se_m_twister_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 25
	.byte		VOL   , 74*se_m_twister_mvl/mxv
	.byte		TIE   , Gs4 , v012
	.byte	W03
	.byte		VOL   , 92*se_m_twister_mvl/mxv
	.byte	W03
	.byte		        98*se_m_twister_mvl/mxv
	.byte	W03
	.byte		        105*se_m_twister_mvl/mxv
	.byte	W03
	.byte		        116*se_m_twister_mvl/mxv
	.byte	W03
	.byte		        127*se_m_twister_mvl/mxv
	.byte	W09
	.byte	W24
	.byte	W24
	.byte	W24
@ 001   ----------------------------------------
	.byte	W21
	.byte		        114*se_m_twister_mvl/mxv
	.byte	W03
	.byte	W07
	.byte		        108*se_m_twister_mvl/mxv
	.byte	W08
	.byte		        98*se_m_twister_mvl/mxv
	.byte	W07
	.byte		        88*se_m_twister_mvl/mxv
	.byte	W02
	.byte	W04
	.byte		        69*se_m_twister_mvl/mxv
	.byte	W06
	.byte		        50*se_m_twister_mvl/mxv
	.byte	W04
	.byte		        37*se_m_twister_mvl/mxv
	.byte	W05
	.byte		        20*se_m_twister_mvl/mxv
	.byte	W05
	.byte		EOT   
	.byte	FINE

@******************************************************@
	.align	2

se_m_twister:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_twister_pri	@ Priority
	.byte	se_m_twister_rev	@ Reverb.

	.word	se_m_twister_grp

	.word	se_m_twister_1
	.word	se_m_twister_2

	.end
