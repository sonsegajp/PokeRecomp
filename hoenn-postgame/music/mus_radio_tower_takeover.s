	.include "MPlayDef.s"

	.equ	mus_radio_tower_takeover_grp, voicegroup113
	.equ	mus_radio_tower_takeover_pri, 0
	.equ	mus_radio_tower_takeover_rev, reverb_set+50
	.equ	mus_radio_tower_takeover_mvl, 79
	.equ	mus_radio_tower_takeover_key, 0
	.equ	mus_radio_tower_takeover_tbs, 1
	.equ	mus_radio_tower_takeover_exg, 1
	.equ	mus_radio_tower_takeover_cmp, 1

	.section .rodata
	.global	mus_radio_tower_takeover
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

mus_radio_tower_takeover_1:
	.byte	KEYSH , mus_radio_tower_takeover_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 152*mus_radio_tower_takeover_tbs/2
	.byte		VOICE , 21
	.byte		VOL   , 127*mus_radio_tower_takeover_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N06   , Cn4 , v112
	.byte	W06
	.byte		        Dn4 
	.byte	W06
mus_radio_tower_takeover_1_B1:
@ 002   ----------------------------------------
	.byte		N06   , Ds4 , v112
	.byte	W12
	.byte		        Dn4 
	.byte	W12
	.byte		        Cs4 
	.byte	W12
	.byte		        Cn4 
	.byte	W24
	.byte		        Fn2 
	.byte	W06
	.byte		        Gs2 
	.byte	W06
	.byte		        Bn2 
	.byte	W06
	.byte		        Dn3 
	.byte	W06
	.byte		        Fn3 
	.byte	W06
	.byte		        Gs3 
	.byte	W06
@ 003   ----------------------------------------
	.byte	W12
	.byte		        Ds3 
	.byte	W36
	.byte		N24   
	.byte	W24
	.byte		N12   , An2 
	.byte	W12
	.byte		N06   , Ds3 
	.byte	W06
	.byte		        Fn3 
	.byte	W06
@ 004   ----------------------------------------
	.byte		N36   , Fs3 
	.byte	W36
	.byte		N06   , Ds3 
	.byte	W24
	.byte		        Fs3 
	.byte	W12
	.byte		        Ds3 
	.byte	W12
	.byte		        Fs4 
	.byte	W12
@ 005   ----------------------------------------
	.byte	W12
	.byte		        Ds4 
	.byte	W36
	.byte		N48   , An3 
	.byte	W48
@ 006   ----------------------------------------
	.byte		N36   , As3 
	.byte	W36
	.byte		N06   , An3 
	.byte	W24
	.byte		        As3 
	.byte	W12
	.byte		        An3 
	.byte	W12
	.byte		        Fs4 
	.byte	W12
@ 007   ----------------------------------------
mus_radio_tower_takeover_1_007:
	.byte	W12
	.byte		N06   , Bn2 , v112
	.byte	W48
	.byte		        Cn3 
	.byte	W36
	.byte	PEND
@ 008   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_1_007
@ 009   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_1_007
@ 010   ----------------------------------------
	.byte	W96
@ 011   ----------------------------------------
mus_radio_tower_takeover_1_011:
	.byte	W21
	.byte		N03   , Bn3 , v112
	.byte	W03
	.byte		N06   , Cn4 
	.byte	W24
	.byte		N06   
	.byte	W12
	.byte		        Gn3 
	.byte	W24
	.byte		N36   , Ds4 
	.byte	W12
	.byte	PEND
@ 012   ----------------------------------------
mus_radio_tower_takeover_1_012:
	.byte	W24
	.byte		N24   , Dn4 , v112
	.byte	W24
	.byte		N06   , As3 
	.byte	W24
	.byte		        Gn3 
	.byte	W12
	.byte		        An3 
	.byte	W12
	.byte	PEND
@ 013   ----------------------------------------
	.byte		        As3 
	.byte	W36
	.byte		N03   , Fn4 
	.byte	W03
	.byte		N09   , Fs4 
	.byte	W09
	.byte		N06   , Fn4 
	.byte	W12
	.byte		        Ds4 
	.byte	W12
	.byte		        Cn4 
	.byte	W06
	.byte		        Dn4 
	.byte	W06
	.byte		        Ds4 
	.byte	W06
	.byte		        Cn4 
	.byte	W06
@ 014   ----------------------------------------
	.byte		        As3 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
	.byte		        As3 
	.byte	W06
	.byte		        Cn4 
	.byte	W18
	.byte		N12   , Dn4 
	.byte	W24
	.byte		        Ds4 
	.byte	W24
	.byte		N06   
	.byte	W06
	.byte		        Fs4 
	.byte	W06
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_1_011
@ 016   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_1_012
@ 017   ----------------------------------------
	.byte		N06   , As3 , v112
	.byte	W36
	.byte		N03   , Fs3 
	.byte	W03
	.byte		N09   , Gn3 
	.byte	W09
	.byte		N06   , Fn3 
	.byte	W12
	.byte		        Ds3 
	.byte	W12
	.byte		        Cn3 
	.byte	W06
	.byte		        Dn3 
	.byte	W06
	.byte		        Ds3 
	.byte	W06
	.byte		        Cn3 
	.byte	W06
@ 018   ----------------------------------------
	.byte		        As2 
	.byte	W06
	.byte		        Gn2 
	.byte	W06
	.byte		        As2 
	.byte	W06
	.byte		        Cn3 
	.byte	W18
	.byte		N12   , Dn3 
	.byte	W24
	.byte		        Ds3 
	.byte	W12
	.byte		N06   , Fn3 
	.byte	W06
	.byte		        En3 
	.byte	W06
	.byte		        Ds3 
	.byte	W06
	.byte		        Dn3 
	.byte	W06
@ 019   ----------------------------------------
mus_radio_tower_takeover_1_019:
	.byte	W12
	.byte		N06   , Fn3 , v112
	.byte	W48
	.byte		        Fs3 
	.byte	W36
	.byte	PEND
@ 020   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_1_019
@ 021   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_1_019
@ 022   ----------------------------------------
	.byte	W12
	.byte		N06   , Fn3 , v112
	.byte	W48
	.byte		        Fs3 
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		        Ds3 
	.byte	W06
	.byte		        Cn3 
	.byte	W06
@ 023   ----------------------------------------
	.byte		        Fs3 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Fs3 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Fs3 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Fs3 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
	.byte		        As3 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
	.byte		        As3 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
	.byte		        As3 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
	.byte		        As3 
	.byte	W06
@ 024   ----------------------------------------
	.byte		        Gs3 
	.byte	W06
	.byte		        Bn3 
	.byte	W06
	.byte		        Gs3 
	.byte	W06
	.byte		        Bn3 
	.byte	W06
	.byte		        Gs3 
	.byte	W06
	.byte		        Bn3 
	.byte	W06
	.byte		        Gs3 
	.byte	W06
	.byte		        Bn3 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Cn4 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Cn4 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Cn4 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Cn4 
	.byte	W06
@ 025   ----------------------------------------
	.byte	W96
@ 026   ----------------------------------------
	.byte	W72
	.byte		N12   , Fn4 
	.byte	W12
	.byte		N06   , Cn4 
	.byte	W06
	.byte		        Dn4 
	.byte	W06
	.byte	GOTO
	 .word	mus_radio_tower_takeover_1_B1
mus_radio_tower_takeover_1_B2:
@ 027   ----------------------------------------
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

mus_radio_tower_takeover_2:
	.byte	KEYSH , mus_radio_tower_takeover_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 21
	.byte		VOL   , 127*mus_radio_tower_takeover_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N06   , Cn5 , v112
	.byte	W06
	.byte		        Dn5 
	.byte	W06
mus_radio_tower_takeover_2_B1:
@ 001   ----------------------------------------
	.byte		N06   , Ds5 , v112
	.byte	W12
	.byte		        Dn5 
	.byte	W12
	.byte		        Cs5 
	.byte	W12
	.byte		        Cn5 
	.byte	W24
	.byte		N36   , Bn3 
	.byte	W36
@ 002   ----------------------------------------
	.byte	W12
	.byte		N06   , Gn3 
	.byte	W36
	.byte		N48   , An3 
	.byte	W48
@ 003   ----------------------------------------
	.byte		N36   , As3 
	.byte	W36
	.byte		N06   , An3 
	.byte	W24
	.byte		        As3 
	.byte	W12
	.byte		        An3 
	.byte	W12
	.byte		        Cn5 
	.byte	W12
@ 004   ----------------------------------------
	.byte	W12
	.byte		        Gn3 
	.byte	W36
	.byte		N24   , Fn4 
	.byte	W24
	.byte		N06   , Cn4 
	.byte	W06
	.byte		        Dn4 
	.byte	W06
	.byte		        Ds4 
	.byte	W06
	.byte		        Fn4 
	.byte	W06
@ 005   ----------------------------------------
	.byte		N36   , Fs4 
	.byte	W36
	.byte		N06   , Ds4 
	.byte	W24
	.byte		        Fs4 
	.byte	W12
	.byte		        Ds4 
	.byte	W12
	.byte		        Ds5 
	.byte	W12
@ 006   ----------------------------------------
mus_radio_tower_takeover_2_006:
	.byte	W12
	.byte		N06   , Fn3 , v112
	.byte	W48
	.byte		        Fs3 
	.byte	W36
	.byte	PEND
@ 007   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_2_006
@ 008   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_2_006
@ 009   ----------------------------------------
	.byte		N06   , Gs3 , v112
	.byte	W06
	.byte		        Fn3 
	.byte	W06
	.byte		        Dn3 
	.byte	W06
	.byte		        Bn2 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Fs3 
	.byte	W06
	.byte		        Ds3 
	.byte	W06
	.byte		        Cn3 
	.byte	W06
	.byte		        As3 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
	.byte		        En3 
	.byte	W06
	.byte		        Cs3 
	.byte	W06
	.byte		        Bn3 
	.byte	W06
	.byte		        Gs3 
	.byte	W06
	.byte		        Fn3 
	.byte	W06
	.byte		        Dn3 
	.byte	W06
@ 010   ----------------------------------------
mus_radio_tower_takeover_2_010:
	.byte	W21
	.byte		N03   , Dn4 , v112
	.byte	W03
	.byte		N06   , Ds4 
	.byte	W24
	.byte		N06   
	.byte	W12
	.byte		        Cn4 
	.byte	W24
	.byte		N36   , Fs4 
	.byte	W12
	.byte	PEND
@ 011   ----------------------------------------
mus_radio_tower_takeover_2_011:
	.byte	W24
	.byte		N24   , Fn4 , v112
	.byte	W24
	.byte		N06   , Ds4 
	.byte	W24
	.byte		        Cn4 
	.byte	W12
	.byte		        Dn4 
	.byte	W12
	.byte	PEND
@ 012   ----------------------------------------
	.byte		        Ds4 
	.byte	W12
	.byte		        Ds3 
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N03   , An4 
	.byte	W03
	.byte		TIE   , As4 
	.byte	W56
	.byte	W01
@ 013   ----------------------------------------
	.byte	W72
	.byte		EOT   
	.byte	W12
	.byte		N06   , Cn5 
	.byte	W06
	.byte		        Dn5 
	.byte	W06
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_2_010
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_2_011
@ 016   ----------------------------------------
	.byte		N06   , Ds4 , v112
	.byte	W12
	.byte		        Ds3 
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N03   , Bn3 
	.byte	W03
	.byte		TIE   , Cn4 
	.byte	W56
	.byte	W01
@ 017   ----------------------------------------
	.byte	W72
	.byte		EOT   
	.byte		N06   , As3 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Gs3 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
@ 018   ----------------------------------------
	.byte		N03   , Bn3 
	.byte	W03
	.byte		N09   , Cn4 
	.byte	W09
	.byte		N12   , Ds4 
	.byte	W12
	.byte		        Cn4 
	.byte	W12
	.byte		N24   , Fs4 
	.byte	W24
	.byte		        Fn4 
	.byte	W24
	.byte		N12   , Cn4 
	.byte	W12
@ 019   ----------------------------------------
	.byte		N03   , Dn4 
	.byte	W03
	.byte		N24   , Ds4 
	.byte	W24
	.byte	W03
	.byte		N03   , Dn4 
	.byte	W03
	.byte		        Cs4 
	.byte	W03
	.byte		N36   , Cn4 
	.byte	W36
	.byte		N06   , Fn4 
	.byte	W12
	.byte		        Fs4 
	.byte	W12
@ 020   ----------------------------------------
	.byte		N12   , Cn4 
	.byte	W12
	.byte		        Ds4 
	.byte	W12
	.byte		        Cn4 
	.byte	W12
	.byte		N24   , Fn4 
	.byte	W24
	.byte		        Fs4 
	.byte	W24
	.byte		N12   , Cn4 
	.byte	W12
@ 021   ----------------------------------------
	.byte		N06   , Bn3 
	.byte	W12
	.byte		        Cn4 
	.byte	W12
	.byte		        Dn4 
	.byte	W12
	.byte		N24   , Ds4 
	.byte	W36
	.byte		N06   , Dn4 
	.byte	W12
	.byte		        Cn4 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
@ 022   ----------------------------------------
	.byte		N03   , Bn3 
	.byte	W03
	.byte		N44   , Cn4 
	.byte	W44
	.byte	W01
	.byte		N48   , Cs4 
	.byte	W48
@ 023   ----------------------------------------
	.byte		        Dn4 
	.byte	W48
	.byte		        Ds4 
	.byte	W48
@ 024   ----------------------------------------
	.byte		N06   , En4 
	.byte	W06
	.byte		        Cs4 
	.byte	W06
	.byte		        As3 
	.byte	W06
	.byte		        Gn3 
	.byte	W06
	.byte		        Fn4 
	.byte	W06
	.byte		        Dn4 
	.byte	W06
	.byte		        Bn3 
	.byte	W06
	.byte		        Gs3 
	.byte	W06
	.byte		        Fs4 
	.byte	W06
	.byte		        Ds4 
	.byte	W06
	.byte		        Cn4 
	.byte	W06
	.byte		        An3 
	.byte	W06
	.byte		        Gn4 
	.byte	W06
	.byte		        En4 
	.byte	W06
	.byte		        Cs4 
	.byte	W06
	.byte		        As3 
	.byte	W06
@ 025   ----------------------------------------
	.byte		        Gs4 
	.byte	W06
	.byte		        Fn4 
	.byte	W06
	.byte		        Dn4 
	.byte	W06
	.byte		        Bn3 
	.byte	W06
	.byte		        An4 
	.byte	W06
	.byte		        Fs4 
	.byte	W06
	.byte		        Ds4 
	.byte	W06
	.byte		        Cn4 
	.byte	W06
	.byte		        As4 
	.byte	W06
	.byte		        Gn4 
	.byte	W06
	.byte		        En4 
	.byte	W06
	.byte		        Cs4 
	.byte	W06
	.byte		N12   , Bn4 
	.byte	W12
	.byte		N06   , Cn5 
	.byte	W06
	.byte		        Dn5 
	.byte	W06
	.byte	GOTO
	 .word	mus_radio_tower_takeover_2_B1
mus_radio_tower_takeover_2_B2:
@ 026   ----------------------------------------
	.byte	FINE

@**************** Track 3 (Midi-Chn.3) ****************@

mus_radio_tower_takeover_3:
	.byte	KEYSH , mus_radio_tower_takeover_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 21
	.byte		VOL   , 127*mus_radio_tower_takeover_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W12
mus_radio_tower_takeover_3_B1:
@ 001   ----------------------------------------
	.byte		N06   , Fn2 , v112
	.byte	W12
	.byte		        Gs2 
	.byte	W12
	.byte		        Cn3 
	.byte	W12
	.byte		        Dn3 
	.byte	W24
	.byte		N36   , Dn2 
	.byte	W36
@ 002   ----------------------------------------
mus_radio_tower_takeover_3_002:
	.byte		N06   , Cn2 , v112
	.byte	W12
	.byte		        Cn3 
	.byte	W12
	.byte		        Cn2 
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		        Cn3 
	.byte	W12
	.byte		        Cn2 
	.byte	W12
	.byte		        Ds2 
	.byte	W12
	.byte		        Gn2 
	.byte	W12
	.byte	PEND
@ 003   ----------------------------------------
mus_radio_tower_takeover_3_003:
	.byte		N06   , Cn2 , v112
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		        Fs2 
	.byte	W12
	.byte		        An2 
	.byte	W12
	.byte	PEND
@ 004   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_002
@ 005   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_003
@ 006   ----------------------------------------
mus_radio_tower_takeover_3_006:
	.byte		N06   , Gs1 , v112
	.byte	W12
	.byte		        Gs2 
	.byte	W12
	.byte		        Gs1 
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		        An1 
	.byte	W12
	.byte		        An2 
	.byte	W12
	.byte		        Gs2 
	.byte	W06
	.byte		        Fs2 
	.byte	W06
	.byte		        Ds2 
	.byte	W06
	.byte		        Cn2 
	.byte	W06
	.byte	PEND
@ 007   ----------------------------------------
mus_radio_tower_takeover_3_007:
	.byte		N06   , Gs1 , v112
	.byte	W12
	.byte		        Gs2 
	.byte	W12
	.byte		        Gs1 
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		        An1 
	.byte	W12
	.byte		        An2 
	.byte	W12
	.byte		        Cn2 
	.byte	W12
	.byte		        Fs2 
	.byte	W12
	.byte	PEND
@ 008   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_006
@ 009   ----------------------------------------
	.byte		N06   , Gs1 , v112
	.byte	W12
	.byte		        Fn2 
	.byte	W12
	.byte		        An1 
	.byte	W12
	.byte		        Fs2 
	.byte	W12
	.byte		        As1 
	.byte	W12
	.byte		        Gn2 
	.byte	W12
	.byte		        Bn1 
	.byte	W12
	.byte		        Gs2 
	.byte	W12
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_002
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_003
@ 012   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_002
@ 013   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_003
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_002
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_003
@ 016   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_002
@ 017   ----------------------------------------
	.byte		N06   , Cn2 , v112
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		        Dn2 
	.byte	W06
	.byte		        Ds2 
	.byte	W06
	.byte		        En2 
	.byte	W06
	.byte		        Fn2 
	.byte	W06
@ 018   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_006
@ 019   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_007
@ 020   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_006
@ 021   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_3_007
@ 022   ----------------------------------------
	.byte		N06   , Cn2 , v112
	.byte	W12
	.byte		        Cn3 
	.byte	W12
	.byte		        Cn2 
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		        Cs2 
	.byte	W12
	.byte		        Cs3 
	.byte	W12
	.byte		        As2 
	.byte	W06
	.byte		        Gn2 
	.byte	W06
	.byte		        En2 
	.byte	W06
	.byte		        Cs2 
	.byte	W06
@ 023   ----------------------------------------
	.byte		        Dn2 
	.byte	W12
	.byte		        Dn3 
	.byte	W12
	.byte		        Dn2 
	.byte	W12
	.byte		N06   
	.byte	W12
	.byte		        Ds2 
	.byte	W12
	.byte		        Ds3 
	.byte	W12
	.byte		        Cn3 
	.byte	W06
	.byte		        An2 
	.byte	W06
	.byte		        Fs2 
	.byte	W06
	.byte		        Ds2 
	.byte	W06
@ 024   ----------------------------------------
	.byte		        En2 
	.byte	W12
	.byte		        Cs3 
	.byte	W12
	.byte		        Fn2 
	.byte	W12
	.byte		        Dn3 
	.byte	W12
	.byte		        Fs2 
	.byte	W12
	.byte		        Ds3 
	.byte	W12
	.byte		        Gn2 
	.byte	W12
	.byte		        En3 
	.byte	W12
@ 025   ----------------------------------------
	.byte		        Gs2 
	.byte	W12
	.byte		        Fn3 
	.byte	W12
	.byte		        An2 
	.byte	W12
	.byte		        Fs3 
	.byte	W12
	.byte		        As2 
	.byte	W12
	.byte		        Gn3 
	.byte	W12
	.byte		N12   , Bn2 
	.byte	W12
	.byte		N06   , Gs3 
	.byte	W12
	.byte	GOTO
	 .word	mus_radio_tower_takeover_3_B1
mus_radio_tower_takeover_3_B2:
@ 026   ----------------------------------------
	.byte	FINE

@**************** Track 4 (Midi-Chn.4) ****************@

mus_radio_tower_takeover_4:
	.byte	KEYSH , mus_radio_tower_takeover_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 126
	.byte		PAN   , c_v+0
	.byte		VOL   , 66*mus_radio_tower_takeover_mvl/mxv
	.byte	W12
mus_radio_tower_takeover_4_B1:
@ 001   ----------------------------------------
	.byte	W96
@ 002   ----------------------------------------
mus_radio_tower_takeover_4_002:
	.byte	W12
	.byte		N12   , Gn5 , v032
	.byte	W24
	.byte		        Gn5 , v040
	.byte	W24
	.byte		        Gn5 , v036
	.byte	W24
	.byte		N12   
	.byte	W12
	.byte	PEND
@ 003   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 004   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 005   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 006   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 007   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 008   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 009   ----------------------------------------
	.byte	W96
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 012   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 013   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 016   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 017   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 018   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 019   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 020   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 021   ----------------------------------------
	.byte	W12
	.byte		N12   , Gn5 , v032
	.byte	W24
	.byte		        Gn5 , v040
	.byte	W24
	.byte		        Gn5 , v036
	.byte	W36
@ 022   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 023   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_4_002
@ 024   ----------------------------------------
	.byte	W96
@ 025   ----------------------------------------
	.byte	W96
	.byte	GOTO
	 .word	mus_radio_tower_takeover_4_B1
mus_radio_tower_takeover_4_B2:
@ 026   ----------------------------------------
	.byte	FINE

@**************** Track 5 (Midi-Chn.5) ****************@

mus_radio_tower_takeover_5:
	.byte	KEYSH , mus_radio_tower_takeover_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 0
	.byte		VOL   , 127*mus_radio_tower_takeover_mvl/mxv
	.byte		PAN   , c_v+4
	.byte	W12
mus_radio_tower_takeover_5_B1:
@ 001   ----------------------------------------
	.byte		N48   , An2 , v112
	.byte	W48
	.byte		N12   , En1 
	.byte	W12
	.byte		        Cn1 
	.byte	W12
	.byte		N06   , En1 , v056
	.byte	W06
	.byte		N06   
	.byte	W06
	.byte		        En1 , v068
	.byte	W06
	.byte		N06   
	.byte	W06
@ 002   ----------------------------------------
mus_radio_tower_takeover_5_002:
	.byte		N12   , Cn1 , v112
	.byte	W24
	.byte		        En1 
	.byte	W24
	.byte		        Cn1 
	.byte	W24
	.byte		        En1 
	.byte	W24
	.byte	PEND
@ 003   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 004   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 005   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 006   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 007   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 008   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 009   ----------------------------------------
	.byte		N48   , An2 , v112
	.byte	W48
	.byte		N12   , En1 
	.byte	W12
	.byte		        Cn1 
	.byte	W12
	.byte		N06   , En1 
	.byte	W06
	.byte		N06   
	.byte	W06
	.byte		N06   
	.byte	W06
	.byte		N06   
	.byte	W06
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 012   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 013   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 016   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 017   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 018   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 019   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 020   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 021   ----------------------------------------
	.byte		N12   , Cn1 , v112
	.byte	W24
	.byte		        En1 
	.byte	W24
	.byte		        Cn1 
	.byte	W24
	.byte		N06   , En1 
	.byte	W12
	.byte		N06   
	.byte	W06
	.byte		N06   
	.byte	W06
@ 022   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 023   ----------------------------------------
	.byte	PATT
	 .word	mus_radio_tower_takeover_5_002
@ 024   ----------------------------------------
	.byte		N48   , An2 , v112
	.byte	W48
	.byte		N12   , En1 
	.byte	W12
	.byte		        Cn1 
	.byte	W12
	.byte		N06   , En1 , v048
	.byte	W06
	.byte		N06   
	.byte	W06
	.byte		        En1 , v072
	.byte	W06
	.byte		N06   
	.byte	W06
@ 025   ----------------------------------------
	.byte		N48   , An2 , v108
	.byte	W48
	.byte		N12   , En1 
	.byte	W12
	.byte		        Cn1 
	.byte	W12
	.byte		N06   , En1 
	.byte	W12
	.byte		N06   
	.byte	W06
	.byte		N06   
	.byte	W06
	.byte	GOTO
	 .word	mus_radio_tower_takeover_5_B1
mus_radio_tower_takeover_5_B2:
@ 026   ----------------------------------------
	.byte	FINE

@******************************************************@
	.align	2

mus_radio_tower_takeover:
	.byte	5	@ NumTrks
	.byte	0	@ NumBlks
	.byte	mus_radio_tower_takeover_pri	@ Priority
	.byte	mus_radio_tower_takeover_rev	@ Reverb.

	.word	mus_radio_tower_takeover_grp

	.word	mus_radio_tower_takeover_1
	.word	mus_radio_tower_takeover_2
	.word	mus_radio_tower_takeover_3
	.word	mus_radio_tower_takeover_4
	.word	mus_radio_tower_takeover_5

	.end
