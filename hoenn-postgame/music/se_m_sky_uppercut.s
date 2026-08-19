	.include "MPlayDef.s"

	.equ	se_m_sky_uppercut_grp, voicegroup111
	.equ	se_m_sky_uppercut_pri, 4
	.equ	se_m_sky_uppercut_rev, reverb_set+50
	.equ	se_m_sky_uppercut_mvl, 110
	.equ	se_m_sky_uppercut_key, 0
	.equ	se_m_sky_uppercut_tbs, 1
	.equ	se_m_sky_uppercut_exg, 1
	.equ	se_m_sky_uppercut_cmp, 1

	.section .rodata
	.global	se_m_sky_uppercut
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_sky_uppercut_1:
	.byte	KEYSH , se_m_sky_uppercut_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 220*se_m_sky_uppercut_tbs/2
	.byte		VOICE , 41
	.byte		VOL   , 127*se_m_sky_uppercut_mvl/mxv
	.byte		BENDR , 12
	.byte		PAN   , c_v+6
	.byte		BEND  , c_v+0
	.byte		N02   , Cn4 , v127
	.byte	W03
	.byte		VOL   , 121*se_m_sky_uppercut_mvl/mxv
	.byte	W03
	.byte		        108*se_m_sky_uppercut_mvl/mxv
	.byte		PAN   , c_v-6
	.byte		N02   , Cn5 
	.byte	W03
	.byte		VOL   , 127*se_m_sky_uppercut_mvl/mxv
	.byte	W01
	.byte		VOICE , 22
	.byte		PAN   , c_v+0
	.byte		N24   , Cn4 , v060
	.byte	W02
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W04
	.byte		        c_v+1
	.byte	W03
	.byte		        c_v+1
	.byte	W01
	.byte		VOL   , 122*se_m_sky_uppercut_mvl/mxv
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W01
	.byte		VOL   , 119*se_m_sky_uppercut_mvl/mxv
	.byte	W02
	.byte		        106*se_m_sky_uppercut_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		VOL   , 89*se_m_sky_uppercut_mvl/mxv
	.byte		BEND  , c_v-1
	.byte	W02
	.byte		VOL   , 71*se_m_sky_uppercut_mvl/mxv
	.byte		BEND  , c_v-2
	.byte	W01
	.byte		VOL   , 44*se_m_sky_uppercut_mvl/mxv
	.byte	W01
	.byte		        18*se_m_sky_uppercut_mvl/mxv
	.byte		BEND  , c_v-3
	.byte	W02
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_m_sky_uppercut_2:
	.byte	KEYSH , se_m_sky_uppercut_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 4
	.byte		VOL   , 127*se_m_sky_uppercut_mvl/mxv
	.byte		N02   , Cn3 , v060
	.byte	W03
	.byte		        Gn2 
	.byte	W03
	.byte		        Gs4 
	.byte	W04
	.byte		        Gs4 , v020
	.byte	W02
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	W06
	.byte	FINE

@******************************************************@
	.align	2

se_m_sky_uppercut:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_sky_uppercut_pri	@ Priority
	.byte	se_m_sky_uppercut_rev	@ Reverb.

	.word	se_m_sky_uppercut_grp

	.word	se_m_sky_uppercut_1
	.word	se_m_sky_uppercut_2

	.end
