	.include "MPlayDef.s"

	.equ	se_m_rain_dance_grp, voicegroup111
	.equ	se_m_rain_dance_pri, 4
	.equ	se_m_rain_dance_rev, reverb_set+50
	.equ	se_m_rain_dance_mvl, 110
	.equ	se_m_rain_dance_key, 0
	.equ	se_m_rain_dance_tbs, 1
	.equ	se_m_rain_dance_exg, 1
	.equ	se_m_rain_dance_cmp, 1

	.section .rodata
	.global	se_m_rain_dance
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_rain_dance_1:
	.byte	KEYSH , se_m_rain_dance_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 220*se_m_rain_dance_tbs/2
	.byte		VOICE , 36
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 24*se_m_rain_dance_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		TIE   , Gn3 , v064
	.byte	W03
	.byte		VOL   , 43*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        62*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        70*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        82*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        94*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        108*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        127*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+3
	.byte	W06
	.byte		        c_v+11
	.byte	W12
	.byte		        c_v+20
	.byte	W06
	.byte		        c_v+25
	.byte	W12
	.byte		        c_v+14
	.byte	W06
	.byte		        c_v+8
	.byte	W06
	.byte	W06
	.byte		        c_v-3
	.byte	W12
	.byte		        c_v-16
	.byte	W06
@ 002   ----------------------------------------
	.byte	W06
	.byte		        c_v-26
	.byte	W12
	.byte		        c_v-16
	.byte	W06
	.byte		VOL   , 121*se_m_rain_dance_mvl/mxv
	.byte		PAN   , c_v-5
	.byte	W03
	.byte		VOL   , 117*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        114*se_m_rain_dance_mvl/mxv
	.byte		PAN   , c_v+6
	.byte	W03
	.byte		VOL   , 108*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        104*se_m_rain_dance_mvl/mxv
	.byte		PAN   , c_v+14
	.byte	W03
	.byte		VOL   , 100*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        96*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        89*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        85*se_m_rain_dance_mvl/mxv
	.byte		PAN   , c_v+24
	.byte	W03
	.byte		VOL   , 77*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        67*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        51*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        37*se_m_rain_dance_mvl/mxv
	.byte		PAN   , c_v+15
	.byte	W03
	.byte		VOL   , 19*se_m_rain_dance_mvl/mxv
	.byte	W03
	.byte		        2*se_m_rain_dance_mvl/mxv
	.byte		PAN   , c_v+4
	.byte	W06
	.byte		EOT   
	.byte	FINE

@******************************************************@
	.align	2

se_m_rain_dance:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_rain_dance_pri	@ Priority
	.byte	se_m_rain_dance_rev	@ Reverb.

	.word	se_m_rain_dance_grp

	.word	se_m_rain_dance_1

	.end
