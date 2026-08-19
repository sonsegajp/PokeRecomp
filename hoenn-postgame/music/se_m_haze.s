	.include "MPlayDef.s"

	.equ	se_m_haze_grp, voicegroup111
	.equ	se_m_haze_pri, 4
	.equ	se_m_haze_rev, reverb_set+50
	.equ	se_m_haze_mvl, 110
	.equ	se_m_haze_key, 0
	.equ	se_m_haze_tbs, 1
	.equ	se_m_haze_exg, 1
	.equ	se_m_haze_cmp, 1

	.section .rodata
	.global	se_m_haze
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_haze_1:
	.byte	KEYSH , se_m_haze_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 190*se_m_haze_tbs/2
	.byte		VOICE , 45
	.byte		BENDR , 8
	.byte		PAN   , c_v-47
	.byte		VOL   , 12*se_m_haze_mvl/mxv
	.byte		MOD   , 30
	.byte		BEND  , c_v+1
	.byte		TIE   , As2 , v100
	.byte	W03
	.byte		VOL   , 17*se_m_haze_mvl/mxv
	.byte	W03
	.byte		        26*se_m_haze_mvl/mxv
	.byte		PAN   , c_v-45
	.byte	W02
	.byte		VOL   , 36*se_m_haze_mvl/mxv
	.byte	W02
	.byte		        46*se_m_haze_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-43
	.byte	W02
	.byte		VOL   , 52*se_m_haze_mvl/mxv
	.byte	W02
	.byte		        58*se_m_haze_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-41
	.byte	W01
	.byte		VOL   , 71*se_m_haze_mvl/mxv
	.byte	W02
	.byte		        77*se_m_haze_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-39
	.byte		VOL   , 85*se_m_haze_mvl/mxv
	.byte	W02
	.byte		        91*se_m_haze_mvl/mxv
	.byte	W04
	.byte		        97*se_m_haze_mvl/mxv
	.byte		PAN   , c_v-37
	.byte	W03
	.byte		VOL   , 106*se_m_haze_mvl/mxv
	.byte	W03
	.byte		        114*se_m_haze_mvl/mxv
	.byte		PAN   , c_v-35
	.byte	W02
	.byte		VOL   , 121*se_m_haze_mvl/mxv
	.byte	W04
	.byte		        127*se_m_haze_mvl/mxv
	.byte		PAN   , c_v-32
	.byte	W06
	.byte		        c_v-28
	.byte	W06
	.byte		        c_v-25
	.byte		VOL   , 123*se_m_haze_mvl/mxv
	.byte	W04
	.byte		        121*se_m_haze_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-23
	.byte	W03
	.byte		VOL   , 117*se_m_haze_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-20
	.byte	W03
	.byte		VOL   , 112*se_m_haze_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-17
	.byte	W02
	.byte		VOL   , 106*se_m_haze_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v-13
	.byte	W03
	.byte		VOL   , 102*se_m_haze_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-10
	.byte	W03
	.byte		VOL   , 94*se_m_haze_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-7
	.byte	W04
	.byte		VOL   , 87*se_m_haze_mvl/mxv
	.byte	W02
@ 002   ----------------------------------------
	.byte		PAN   , c_v-3
	.byte	W02
	.byte		VOL   , 86*se_m_haze_mvl/mxv
	.byte	W02
	.byte		        88*se_m_haze_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 92*se_m_haze_mvl/mxv
	.byte	W02
	.byte		        96*se_m_haze_mvl/mxv
	.byte	W03
	.byte	W01
	.byte		        100*se_m_haze_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+2
	.byte		VOL   , 104*se_m_haze_mvl/mxv
	.byte	W03
	.byte		        110*se_m_haze_mvl/mxv
	.byte	W03
	.byte		        114*se_m_haze_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+7
	.byte		VOL   , 118*se_m_haze_mvl/mxv
	.byte	W02
	.byte		        121*se_m_haze_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+10
	.byte		VOL   , 127*se_m_haze_mvl/mxv
	.byte	W06
	.byte		PAN   , c_v+14
	.byte	W06
	.byte		        c_v+16
	.byte	W02
	.byte		VOL   , 123*se_m_haze_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+19
	.byte	W01
	.byte		VOL   , 116*se_m_haze_mvl/mxv
	.byte	W05
	.byte		PAN   , c_v+22
	.byte		VOL   , 109*se_m_haze_mvl/mxv
	.byte	W06
	.byte		PAN   , c_v+25
	.byte		VOL   , 102*se_m_haze_mvl/mxv
	.byte	W04
	.byte		        96*se_m_haze_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+27
	.byte	W06
	.byte		        c_v+29
	.byte		VOL   , 87*se_m_haze_mvl/mxv
	.byte	W06
	.byte		PAN   , c_v+32
	.byte		VOL   , 74*se_m_haze_mvl/mxv
	.byte	W06
	.byte		PAN   , c_v+34
	.byte	W01
	.byte		VOL   , 64*se_m_haze_mvl/mxv
	.byte	W05
	.byte		PAN   , c_v+38
	.byte	W01
	.byte		VOL   , 55*se_m_haze_mvl/mxv
	.byte	W05
@ 003   ----------------------------------------
	.byte		PAN   , c_v+41
	.byte	W01
	.byte		VOL   , 48*se_m_haze_mvl/mxv
	.byte	W05
	.byte		PAN   , c_v+44
	.byte		VOL   , 37*se_m_haze_mvl/mxv
	.byte	W03
	.byte		        29*se_m_haze_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+47
	.byte	W01
	.byte		VOL   , 14*se_m_haze_mvl/mxv
	.byte	W03
	.byte		        7*se_m_haze_mvl/mxv
	.byte	W02
	.byte		EOT   
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_m_haze_2:
	.byte	KEYSH , se_m_haze_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 70
	.byte		BENDR , 8
	.byte		PAN   , c_v+0
	.byte		VOL   , 127*se_m_haze_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N24   , Fn4 , v020
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
	.byte		        Fs4 
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
	.byte		        Gn4 
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
	.byte		        Gs4 
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
@ 001   ----------------------------------------
	.byte		        Gn4 
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
	.byte		        Fs4 
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
	.byte		        Fn4 
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
	.byte		        En4 
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
@ 002   ----------------------------------------
	.byte		N18   , Ds4 
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	FINE

@******************************************************@
	.align	2

se_m_haze:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_haze_pri	@ Priority
	.byte	se_m_haze_rev	@ Reverb.

	.word	se_m_haze_grp

	.word	se_m_haze_1
	.word	se_m_haze_2

	.end
