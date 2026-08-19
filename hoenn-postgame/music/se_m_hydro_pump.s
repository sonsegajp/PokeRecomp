	.include "MPlayDef.s"

	.equ	se_m_hydro_pump_grp, voicegroup111
	.equ	se_m_hydro_pump_pri, 4
	.equ	se_m_hydro_pump_rev, reverb_set+50
	.equ	se_m_hydro_pump_mvl, 115
	.equ	se_m_hydro_pump_key, 0
	.equ	se_m_hydro_pump_tbs, 1
	.equ	se_m_hydro_pump_exg, 1
	.equ	se_m_hydro_pump_cmp, 1

	.section .rodata
	.global	se_m_hydro_pump
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_hydro_pump_1:
	.byte	KEYSH , se_m_hydro_pump_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_hydro_pump_tbs/2
	.byte		VOICE , 36
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 127*se_m_hydro_pump_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		TIE   , Gn2 , v127
	.byte	W06
	.byte		PAN   , c_v-4
	.byte	W06
	.byte		        c_v-10
	.byte	W06
	.byte		        c_v-5
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+4
	.byte	W06
	.byte		        c_v+11
	.byte	W06
	.byte		        c_v+4
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v-4
	.byte	W06
	.byte		        c_v-10
	.byte	W06
	.byte		        c_v-5
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+4
	.byte	W06
	.byte		        c_v+11
	.byte	W02
	.byte		VOL   , 114*se_m_hydro_pump_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+4
	.byte	W01
	.byte		VOL   , 94*se_m_hydro_pump_mvl/mxv
	.byte	W05
@ 002   ----------------------------------------
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 79*se_m_hydro_pump_mvl/mxv
	.byte	W05
	.byte		        58*se_m_hydro_pump_mvl/mxv
	.byte		PAN   , c_v-4
	.byte	W04
	.byte		VOL   , 36*se_m_hydro_pump_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-10
	.byte	W03
	.byte		VOL   , 13*se_m_hydro_pump_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-5
	.byte	W01
	.byte		VOL   , 7*se_m_hydro_pump_mvl/mxv
	.byte	W05
	.byte		EOT   
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_m_hydro_pump_2:
	.byte	KEYSH , se_m_hydro_pump_key+0
@ 000   ----------------------------------------
	.byte		VOL   , 65*se_m_hydro_pump_mvl/mxv
	.byte		N14   , Gn3 , v080
	.byte	W01
	.byte		VOICE , 25
	.byte		VOL   , 84*se_m_hydro_pump_mvl/mxv
	.byte	W01
	.byte		        99*se_m_hydro_pump_mvl/mxv
	.byte	W01
	.byte		        111*se_m_hydro_pump_mvl/mxv
	.byte	W01
	.byte		        127*se_m_hydro_pump_mvl/mxv
	.byte	W06
	.byte		        100*se_m_hydro_pump_mvl/mxv
	.byte	W02
	.byte		        84*se_m_hydro_pump_mvl/mxv
	.byte	W01
	.byte		        65*se_m_hydro_pump_mvl/mxv
	.byte	W11
	.byte	W24
	.byte	W24
	.byte	W24
@ 001   ----------------------------------------
	.byte	W24
	.byte	FINE

@******************************************************@
	.align	2

se_m_hydro_pump:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_hydro_pump_pri	@ Priority
	.byte	se_m_hydro_pump_rev	@ Reverb.

	.word	se_m_hydro_pump_grp

	.word	se_m_hydro_pump_1
	.word	se_m_hydro_pump_2

	.end
