	.include "MPlayDef.s"

	.equ	se_m_charge_grp, voicegroup111
	.equ	se_m_charge_pri, 4
	.equ	se_m_charge_rev, reverb_set+50
	.equ	se_m_charge_mvl, 90
	.equ	se_m_charge_key, 0
	.equ	se_m_charge_tbs, 1
	.equ	se_m_charge_exg, 1
	.equ	se_m_charge_cmp, 1

	.section .rodata
	.global	se_m_charge
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_charge_1:
	.byte	KEYSH , se_m_charge_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_charge_tbs/2
	.byte		VOICE , 18
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 48*se_m_charge_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N66   , Cn5 , v127
	.byte	W01
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		VOL   , 61*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte		BEND  , c_v-2
	.byte	W01
	.byte		VOL   , 82*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W02
	.byte		        c_v-2
	.byte	W01
	.byte		VOL   , 99*se_m_charge_mvl/mxv
	.byte		PAN   , c_v-4
	.byte		BEND  , c_v-4
	.byte	W01
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		VOL   , 108*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte		BEND  , c_v-6
	.byte	W02
	.byte		VOL   , 127*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+4
	.byte	W01
	.byte		        c_v+2
	.byte	W01
	.byte		        c_v+0
	.byte		BEND  , c_v-8
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v-2
	.byte	W02
	.byte		        c_v+0
	.byte		BEND  , c_v-13
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W01
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v-2
	.byte		BEND  , c_v-15
	.byte	W01
	.byte		PAN   , c_v-4
	.byte	W02
	.byte		        c_v-2
	.byte	W01
	.byte		        c_v+0
	.byte		BEND  , c_v-19
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W01
	.byte		        c_v+4
	.byte		BEND  , c_v-22
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte		BEND  , c_v-19
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v-2
	.byte		BEND  , c_v-17
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W02
	.byte		        c_v+0
	.byte		BEND  , c_v-19
	.byte	W01
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		        c_v-4
	.byte	W01
	.byte		        c_v-2
	.byte		BEND  , c_v-22
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W02
	.byte		        c_v+2
	.byte		BEND  , c_v-28
	.byte	W01
	.byte		PAN   , c_v+4
	.byte		BEND  , c_v-24
	.byte	W01
	.byte		PAN   , c_v+2
	.byte	W01
	.byte		VOL   , 119*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+0
	.byte	W02
	.byte		        c_v-2
	.byte	W01
	.byte		VOL   , 111*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte		BEND  , c_v-27
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 99*se_m_charge_mvl/mxv
	.byte		PAN   , c_v-2
	.byte	W02
	.byte		        c_v-4
	.byte	W01
	.byte		VOL   , 78*se_m_charge_mvl/mxv
	.byte		PAN   , c_v-2
	.byte		BEND  , c_v-29
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		VOL   , 65*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+2
	.byte	W01
	.byte		        c_v+4
	.byte		BEND  , c_v-31
	.byte	W02
	.byte		VOL   , 51*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+2
	.byte	W01
	.byte		        c_v+0
	.byte	W01
	.byte		VOL   , 34*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v-36
	.byte	W01
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		VOL   , 19*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v-42
	.byte	W02
	.byte		VOL   , 8*se_m_charge_mvl/mxv
	.byte		PAN   , c_v+2
	.byte	W06
	.byte	FINE

@******************************************************@
	.align	2

se_m_charge:
	.byte	1	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_charge_pri	@ Priority
	.byte	se_m_charge_rev	@ Reverb.

	.word	se_m_charge_grp

	.word	se_m_charge_1

	.end
