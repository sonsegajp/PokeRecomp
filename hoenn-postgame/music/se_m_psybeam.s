	.include "MPlayDef.s"

	.equ	se_m_psybeam_grp, voicegroup111
	.equ	se_m_psybeam_pri, 4
	.equ	se_m_psybeam_rev, reverb_set+50
	.equ	se_m_psybeam_mvl, 110
	.equ	se_m_psybeam_key, 0
	.equ	se_m_psybeam_tbs, 1
	.equ	se_m_psybeam_exg, 1
	.equ	se_m_psybeam_cmp, 1

	.section .rodata
	.global	se_m_psybeam
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_psybeam_1:
	.byte	KEYSH , se_m_psybeam_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 190*se_m_psybeam_tbs/2
	.byte		VOICE , 45
	.byte		BENDR , 8
	.byte		PAN   , c_v+0
	.byte		VOL   , 44*se_m_psybeam_mvl/mxv
	.byte		MOD   , 30
	.byte		BEND  , c_v+1
	.byte		N42   , As2 , v120
	.byte	W01
	.byte		PAN   , c_v+3
	.byte	W01
	.byte		        c_v+5
	.byte		VOL   , 66*se_m_psybeam_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		VOL   , 82*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+5
	.byte	W01
	.byte		        c_v+1
	.byte		VOL   , 95*se_m_psybeam_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v-3
	.byte		VOL   , 109*se_m_psybeam_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v-6
	.byte	W02
	.byte		        c_v-10
	.byte		VOL   , 127*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-6
	.byte	W01
	.byte		        c_v-3
	.byte	W01
	.byte		        c_v+0
	.byte	W02
	.byte	W06
	.byte		        c_v+3
	.byte	W01
	.byte		        c_v+5
	.byte	W01
	.byte		        c_v+9
	.byte	W02
	.byte		        c_v+5
	.byte	W02
	.byte		        c_v+1
	.byte		VOL   , 110*se_m_psybeam_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		        c_v-3
	.byte		VOL   , 96*se_m_psybeam_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v-6
	.byte	W01
	.byte		        c_v-10
	.byte		VOL   , 82*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte	W01
	.byte		PAN   , c_v-6
	.byte		VOL   , 65*se_m_psybeam_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v-3
	.byte	W01
	.byte		VOL   , 44*se_m_psybeam_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+0
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_m_psybeam_2:
	.byte	KEYSH , se_m_psybeam_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 47
	.byte		BENDR , 8
	.byte		PAN   , c_v+0
	.byte		VOL   , 44*se_m_psybeam_mvl/mxv
	.byte		MOD   , 30
	.byte		BEND  , c_v+1
	.byte		N42   , As1 , v040
	.byte	W02
	.byte		VOL   , 66*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte		        82*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte	W01
	.byte		        95*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte		        109*se_m_psybeam_mvl/mxv
	.byte	W03
	.byte		        127*se_m_psybeam_mvl/mxv
	.byte	W06
	.byte	W06
	.byte	W06
	.byte		        110*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte		        96*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte		        82*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte	W01
	.byte		        65*se_m_psybeam_mvl/mxv
	.byte	W02
	.byte		        44*se_m_psybeam_mvl/mxv
	.byte	W03
	.byte	FINE

@******************************************************@
	.align	2

se_m_psybeam:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_psybeam_pri	@ Priority
	.byte	se_m_psybeam_rev	@ Reverb.

	.word	se_m_psybeam_grp

	.word	se_m_psybeam_1
	.word	se_m_psybeam_2

	.end
