	.include "MPlayDef.s"

	.equ	se_m_earthquake_grp, voicegroup111
	.equ	se_m_earthquake_pri, 4
	.equ	se_m_earthquake_rev, reverb_set+50
	.equ	se_m_earthquake_mvl, 110
	.equ	se_m_earthquake_key, 0
	.equ	se_m_earthquake_tbs, 1
	.equ	se_m_earthquake_exg, 1
	.equ	se_m_earthquake_cmp, 1

	.section .rodata
	.global	se_m_earthquake
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

se_m_earthquake_1:
	.byte	KEYSH , se_m_earthquake_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 150*se_m_earthquake_tbs/2
	.byte		VOICE , 26
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 127*se_m_earthquake_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N03   , Cs2 , v127
	.byte	W03
	.byte		PAN   , c_v+6
	.byte		N03   , Cn2 , v120
	.byte	W03
@ 002   ----------------------------------------
	.byte	W01
	.byte		PAN   , c_v-6
	.byte		N03   , Bn1 , v116
	.byte	W03
	.byte		PAN   , c_v+11
	.byte		N03   , Cn2 , v112
	.byte	W02
@ 003   ----------------------------------------
se_m_earthquake_1_003:
	.byte	W02
	.byte		PAN   , c_v-11
	.byte		N03   , Cs2 , v108
	.byte	W04
	.byte	PEND
@ 004   ----------------------------------------
se_m_earthquake_1_004:
	.byte		PAN   , c_v+0
	.byte		N03   , Cn2 , v100
	.byte	W03
	.byte		PAN   , c_v+6
	.byte		N03   , Cs2 , v116
	.byte	W03
	.byte	PEND
@ 005   ----------------------------------------
se_m_earthquake_1_005:
	.byte	W01
	.byte		PAN   , c_v-6
	.byte		N03   , Cn2 , v112
	.byte	W03
	.byte		PAN   , c_v+11
	.byte		N03   , Cs2 , v108
	.byte	W02
	.byte	PEND
@ 006   ----------------------------------------
se_m_earthquake_1_006:
	.byte	W02
	.byte		PAN   , c_v-11
	.byte		N03   , Cn2 , v100
	.byte	W04
	.byte	PEND
@ 007   ----------------------------------------
se_m_earthquake_1_007:
	.byte		PAN   , c_v+0
	.byte		N03   , Cs2 , v116
	.byte	W03
	.byte		PAN   , c_v+6
	.byte		N03   , Cn2 , v112
	.byte	W03
	.byte	PEND
@ 008   ----------------------------------------
se_m_earthquake_1_008:
	.byte	W01
	.byte		PAN   , c_v-6
	.byte		N03   , Cs2 , v108
	.byte	W03
	.byte		PAN   , c_v+11
	.byte		N03   , Cn2 , v100
	.byte	W02
	.byte	PEND
@ 009   ----------------------------------------
se_m_earthquake_1_009:
	.byte	W02
	.byte		PAN   , c_v-11
	.byte		N03   , Cs2 , v116
	.byte	W04
	.byte	PEND
@ 010   ----------------------------------------
	.byte		PAN   , c_v+0
	.byte		N03   , Cn2 , v112
	.byte	W03
	.byte		PAN   , c_v+6
	.byte		N03   , Cs2 , v108
	.byte	W03
@ 011   ----------------------------------------
	.byte	W01
	.byte		PAN   , c_v-6
	.byte		N03   , Cn2 , v100
	.byte	W03
	.byte		PAN   , c_v+11
	.byte		N03   , Cs2 , v116
	.byte	W02
@ 012   ----------------------------------------
	.byte	W02
	.byte		PAN   , c_v-11
	.byte		N03   , Cn2 , v112
	.byte	W04
@ 013   ----------------------------------------
se_m_earthquake_1_013:
	.byte		PAN   , c_v+0
	.byte		N03   , Cs2 , v108
	.byte	W03
	.byte		PAN   , c_v+6
	.byte		N03   , Cn2 , v100
	.byte	W03
	.byte	PEND
@ 014   ----------------------------------------
se_m_earthquake_1_014:
	.byte	W01
	.byte		PAN   , c_v-6
	.byte		N03   , Cs2 , v116
	.byte	W03
	.byte		PAN   , c_v+11
	.byte		N03   , Cn2 , v112
	.byte	W02
	.byte	PEND
@ 015   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_003
@ 016   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_004
@ 017   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_005
@ 018   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_006
@ 019   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_013
@ 020   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_014
@ 021   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_003
@ 022   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_004
@ 023   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_005
@ 024   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_006
@ 025   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_007
@ 026   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_008
@ 027   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_1_009
@ 028   ----------------------------------------
	.byte		VOL   , 123*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N03   , Cn2 , v112
	.byte	W03
	.byte		VOL   , 119*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		N03   , Cs2 , v108
	.byte	W03
@ 029   ----------------------------------------
	.byte	W01
	.byte		VOL   , 112*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v-6
	.byte		N03   , Cn2 , v100
	.byte	W03
	.byte		VOL   , 103*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+11
	.byte		N03   , Cs2 , v116
	.byte	W02
@ 030   ----------------------------------------
	.byte	W02
	.byte		VOL   , 99*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v-11
	.byte		N03   , Cn2 , v112
	.byte	W04
@ 031   ----------------------------------------
	.byte		VOL   , 91*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N03   , Cs2 , v108
	.byte	W03
	.byte		VOL   , 84*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		N03   , Cn2 , v100
	.byte	W03
@ 032   ----------------------------------------
	.byte	W01
	.byte		VOL   , 77*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v-6
	.byte		N03   , Cs2 , v116
	.byte	W03
	.byte		VOL   , 67*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+11
	.byte		N03   , Cn2 , v112
	.byte	W02
@ 033   ----------------------------------------
	.byte	W02
	.byte		VOL   , 54*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v-11
	.byte		N03   , Cs2 , v108
	.byte	W04
@ 034   ----------------------------------------
	.byte		VOL   , 35*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N03   , Cn2 , v100
	.byte	W03
	.byte		VOL   , 14*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+6
	.byte	W03
@ 035   ----------------------------------------
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

se_m_earthquake_2:
	.byte	KEYSH , se_m_earthquake_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 5
	.byte		PAN   , c_v+0
	.byte		VOL   , 127*se_m_earthquake_mvl/mxv
	.byte		N02   , Fn2 , v072
	.byte	W02
	.byte		N01   
	.byte	W01
	.byte		N02   , Gn2 
	.byte	W03
@ 001   ----------------------------------------
	.byte		N01   , Gs2 , v060
	.byte	W06
@ 002   ----------------------------------------
	.byte		VOICE , 27
	.byte		N06   , Bn1 , v080
	.byte	W06
@ 003   ----------------------------------------
se_m_earthquake_2_003:
	.byte		PAN   , c_v+0
	.byte		N06   , Bn1 , v080
	.byte	W06
	.byte	PEND
@ 004   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 005   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 006   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_2_003
@ 007   ----------------------------------------
	.byte		N06   , Bn1 , v080
	.byte	W06
@ 008   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 009   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_2_003
@ 010   ----------------------------------------
	.byte		N06   , Bn1 , v080
	.byte	W06
@ 011   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 012   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_2_003
@ 013   ----------------------------------------
	.byte		N06   , Bn1 , v080
	.byte	W06
@ 014   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 015   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_2_003
@ 016   ----------------------------------------
	.byte		N06   , Bn1 , v080
	.byte	W06
@ 017   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 018   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_2_003
@ 019   ----------------------------------------
	.byte		N06   , Bn1 , v080
	.byte	W06
@ 020   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 021   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_2_003
@ 022   ----------------------------------------
	.byte		N06   , Bn1 , v080
	.byte	W06
@ 023   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 024   ----------------------------------------
	.byte	PATT
	 .word	se_m_earthquake_2_003
@ 025   ----------------------------------------
	.byte		N06   , Bn1 , v080
	.byte	W06
@ 026   ----------------------------------------
	.byte		N06   
	.byte	W06
@ 027   ----------------------------------------
	.byte		VOL   , 123*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N06   
	.byte	W03
	.byte		VOL   , 119*se_m_earthquake_mvl/mxv
	.byte	W03
@ 028   ----------------------------------------
	.byte		N06   
	.byte	W01
	.byte		VOL   , 112*se_m_earthquake_mvl/mxv
	.byte	W03
	.byte		        103*se_m_earthquake_mvl/mxv
	.byte	W02
@ 029   ----------------------------------------
	.byte		N06   
	.byte	W02
	.byte		VOL   , 99*se_m_earthquake_mvl/mxv
	.byte	W04
@ 030   ----------------------------------------
	.byte		        91*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N06   
	.byte	W03
	.byte		VOL   , 84*se_m_earthquake_mvl/mxv
	.byte	W03
@ 031   ----------------------------------------
	.byte		N06   
	.byte	W01
	.byte		VOL   , 77*se_m_earthquake_mvl/mxv
	.byte	W03
	.byte		        67*se_m_earthquake_mvl/mxv
	.byte	W02
@ 032   ----------------------------------------
	.byte		N06   
	.byte	W02
	.byte		VOL   , 54*se_m_earthquake_mvl/mxv
	.byte	W04
@ 033   ----------------------------------------
	.byte		        35*se_m_earthquake_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N06   
	.byte	W03
	.byte		VOL   , 14*se_m_earthquake_mvl/mxv
	.byte	W03
@ 034   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

se_m_earthquake:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_m_earthquake_pri	@ Priority
	.byte	se_m_earthquake_rev	@ Reverb.

	.word	se_m_earthquake_grp

	.word	se_m_earthquake_1
	.word	se_m_earthquake_2

	.end
