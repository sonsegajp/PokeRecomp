	.include "MPlayDef.s"

	.equ	se_mugshot_grp, voicegroup111
	.equ	se_mugshot_pri, 5
	.equ	se_mugshot_rev, reverb_set+50
	.equ	se_mugshot_mvl, 90
	.equ	se_mugshot_key, 0
	.equ	se_mugshot_tbs, 1
	.equ	se_mugshot_exg, 1
	.equ	se_mugshot_cmp, 1

	.section .rodata
	.global	se_mugshot
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_mugshot_1:
	.byte	KEYSH , se_mugshot_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 110*se_mugshot_tbs/2
	.byte		VOICE , 66
	.byte		BENDR , 12
	.byte		VOL   , 127*se_mugshot_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v+0
	.byte		N90   , Gs3 , v040
	.byte	W06
	.byte	W06
	.byte		BEND  , c_v+1
	.byte	W04
	.byte		        c_v+3
	.byte	W02
	.byte	W04
	.byte		        c_v+5
	.byte	W02
	.byte	W04
	.byte		        c_v+8
	.byte	W02
	.byte	W03
	.byte		        c_v+11
	.byte	W03
	.byte	W03
	.byte		        c_v+18
	.byte	W03
	.byte	W02
	.byte		        c_v+24
	.byte	W04
	.byte		        c_v+30
	.byte	W02
	.byte		        c_v+38
	.byte	W02
	.byte		        c_v+43
	.byte	W02
	.byte	W01
	.byte		        c_v+56
	.byte	W02
	.byte		        c_v+63
	.byte	W03
	.byte	W03
	.byte		VOL   , 118*se_mugshot_mvl/mxv
	.byte	W03
	.byte		        106*se_mugshot_mvl/mxv
	.byte	W04
	.byte		        96*se_mugshot_mvl/mxv
	.byte	W02
	.byte	W01
	.byte		        78*se_mugshot_mvl/mxv
	.byte	W04
	.byte		        60*se_mugshot_mvl/mxv
	.byte	W01
	.byte	W03
	.byte		        41*se_mugshot_mvl/mxv
	.byte	W03
	.byte		        24*se_mugshot_mvl/mxv
	.byte	W03
	.byte		        12*se_mugshot_mvl/mxv
	.byte	W03
	.byte	W06
@ 002   ----------------------------------------
	.byte	W06
	.byte	W06
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_mugshot_2:
	.byte	KEYSH , se_mugshot_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 2
	.byte		VOL   , 20*se_mugshot_mvl/mxv
	.byte		PAN   , c_v-1
	.byte		BEND  , c_v+0
	.byte		TIE   , Gn3 , v100
	.byte	W01
	.byte		VOL   , 27*se_mugshot_mvl/mxv
	.byte	W01
	.byte		        43*se_mugshot_mvl/mxv
	.byte	W02
	.byte		        53*se_mugshot_mvl/mxv
	.byte	W02
	.byte	W02
	.byte		        63*se_mugshot_mvl/mxv
	.byte	W04
	.byte		        70*se_mugshot_mvl/mxv
	.byte	W04
	.byte		        75*se_mugshot_mvl/mxv
	.byte	W02
	.byte	W03
	.byte		        82*se_mugshot_mvl/mxv
	.byte	W03
	.byte	W02
	.byte		        91*se_mugshot_mvl/mxv
	.byte	W04
	.byte		        96*se_mugshot_mvl/mxv
	.byte	W03
	.byte		        105*se_mugshot_mvl/mxv
	.byte	W03
	.byte	W01
	.byte		BEND  , c_v+1
	.byte	W01
	.byte		VOL   , 115*se_mugshot_mvl/mxv
	.byte	W04
	.byte		BEND  , c_v+2
	.byte	W01
	.byte		VOL   , 127*se_mugshot_mvl/mxv
	.byte	W03
	.byte		BEND  , c_v+4
	.byte	W02
	.byte	W03
	.byte		        c_v+5
	.byte	W03
	.byte	W01
	.byte		        c_v+6
	.byte	W03
	.byte		        c_v+6
	.byte	W02
	.byte	W02
	.byte		        c_v+8
	.byte	W02
	.byte		        c_v+10
	.byte	W02
	.byte	W01
	.byte		        c_v+14
	.byte	W02
	.byte		        c_v+17
	.byte	W03
	.byte		        c_v+24
	.byte	W02
	.byte		        c_v+30
	.byte	W01
	.byte		        c_v+41
	.byte	W01
	.byte		        c_v+46
	.byte	W02
	.byte		        c_v+55
	.byte	W01
	.byte		        c_v+60
	.byte	W01
	.byte		        c_v+63
	.byte	W01
	.byte		VOL   , 123*se_mugshot_mvl/mxv
	.byte	W03
	.byte		        120*se_mugshot_mvl/mxv
	.byte		PAN   , c_v-4
	.byte	W02
	.byte		VOL   , 111*se_mugshot_mvl/mxv
	.byte		PAN   , c_v+4
	.byte	W02
	.byte		VOL   , 106*se_mugshot_mvl/mxv
	.byte		PAN   , c_v-6
	.byte	W02
	.byte	W01
	.byte		VOL   , 98*se_mugshot_mvl/mxv
	.byte		PAN   , c_v+6
	.byte	W02
	.byte		VOL   , 84*se_mugshot_mvl/mxv
	.byte		PAN   , c_v-20
	.byte	W03
@ 001   ----------------------------------------
	.byte		VOL   , 70*se_mugshot_mvl/mxv
	.byte		PAN   , c_v+22
	.byte	W02
	.byte		VOL   , 58*se_mugshot_mvl/mxv
	.byte		PAN   , c_v-37
	.byte	W02
	.byte		VOL   , 43*se_mugshot_mvl/mxv
	.byte		PAN   , c_v+37
	.byte	W02
	.byte	W01
	.byte		VOL   , 27*se_mugshot_mvl/mxv
	.byte		PAN   , c_v-48
	.byte	W02
	.byte		VOL   , 15*se_mugshot_mvl/mxv
	.byte		PAN   , c_v+48
	.byte	W01
	.byte		VOL   , 6*se_mugshot_mvl/mxv
	.byte	W02
	.byte		EOT   
	.byte	FINE

@******************************************************@
	.align	2

se_mugshot:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_mugshot_pri	@ Priority
	.byte	se_mugshot_rev	@ Reverb.

	.word	se_mugshot_grp

	.word	se_mugshot_1
	.word	se_mugshot_2

	.end
