	.include "MPlayDef.s"

	.equ	mus_weather_kyogre_grp, voicegroup086
	.equ	mus_weather_kyogre_pri, 0
	.equ	mus_weather_kyogre_rev, reverb_set+50
	.equ	mus_weather_kyogre_mvl, 80
	.equ	mus_weather_kyogre_key, 0
	.equ	mus_weather_kyogre_tbs, 1
	.equ	mus_weather_kyogre_exg, 1
	.equ	mus_weather_kyogre_cmp, 1

	.section .rodata
	.global	mus_weather_kyogre
	.align	2

@**************** Track 1 (Midi-Chn.1) ****************@

mus_weather_kyogre_1:
	.byte	KEYSH , mus_weather_kyogre_key+0
@ 000   ----------------------------------------
@ 001   ----------------------------------------
	.byte	TEMPO , 128*mus_weather_kyogre_tbs/2
	.byte		VOICE , 73
	.byte		LFOS  , 44
	.byte		BENDR , 12
	.byte	W24
mus_weather_kyogre_1_B1:
@ 002   ----------------------------------------
mus_weather_kyogre_1_002:
	.byte		PAN   , c_v+0
	.byte		VOL   , 16*mus_weather_kyogre_mvl/mxv
	.byte		TIE   , As3 , v112
	.byte	W48
	.byte		VOL   , 16*mus_weather_kyogre_mvl/mxv
	.byte	W32
	.byte		        18*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        20*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        21*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        23*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte	PEND
@ 003   ----------------------------------------
	.byte		        24*mus_weather_kyogre_mvl/mxv
	.byte	W08
	.byte		        27*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        29*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        34*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        40*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        42*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        47*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        50*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        51*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        56*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        62*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        66*mus_weather_kyogre_mvl/mxv
	.byte		MOD   , 5
	.byte	W48
	.byte		EOT   
@ 004   ----------------------------------------
	.byte		MOD   , 0
	.byte		TIE   , Bn3 
	.byte	W96
@ 005   ----------------------------------------
	.byte		MOD   , 9
	.byte	W12
	.byte		VOL   , 64*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        61*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        58*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        54*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        50*mus_weather_kyogre_mvl/mxv
	.byte	W08
	.byte		        45*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        42*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        39*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        35*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        32*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        29*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        26*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        23*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        20*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        16*mus_weather_kyogre_mvl/mxv
	.byte	W24
	.byte		EOT   
@ 006   ----------------------------------------
	.byte		PAN   , c_v+0
	.byte		VOL   , 16*mus_weather_kyogre_mvl/mxv
	.byte		MOD   , 0
	.byte		TIE   , As3 
	.byte	W48
	.byte		VOL   , 16*mus_weather_kyogre_mvl/mxv
	.byte	W32
	.byte		        18*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        20*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        21*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        23*mus_weather_kyogre_mvl/mxv
	.byte	W04
@ 007   ----------------------------------------
mus_weather_kyogre_1_007:
	.byte		VOL   , 24*mus_weather_kyogre_mvl/mxv
	.byte	W08
	.byte		        27*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        29*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        34*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        40*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        42*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        47*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        50*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        51*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        56*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        62*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        66*mus_weather_kyogre_mvl/mxv
	.byte	W48
	.byte	PEND
	.byte		EOT   , As3 
@ 008   ----------------------------------------
	.byte		TIE   , Bn3 , v112
	.byte	W96
@ 009   ----------------------------------------
mus_weather_kyogre_1_009:
	.byte	W12
	.byte		VOL   , 64*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        61*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        58*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        54*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        50*mus_weather_kyogre_mvl/mxv
	.byte	W08
	.byte		        45*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        42*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        39*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        35*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        32*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        29*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        26*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        23*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        20*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        16*mus_weather_kyogre_mvl/mxv
	.byte	W24
	.byte	PEND
	.byte		EOT   , Bn3 
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_1_002
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_1_007
	.byte		EOT   , As3 
@ 012   ----------------------------------------
	.byte		TIE   , Bn3 , v112
	.byte	W96
@ 013   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_1_009
	.byte		EOT   , Bn3 
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_1_002
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_1_007
	.byte		EOT   , As3 
@ 016   ----------------------------------------
	.byte		TIE   , Bn3 , v112
	.byte	W96
@ 017   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_1_009
	.byte		EOT   , Bn3 
	.byte	GOTO
	 .word	mus_weather_kyogre_1_B1
mus_weather_kyogre_1_B2:
@ 018   ----------------------------------------
	.byte	FINE

@**************** Track 2 (Midi-Chn.2) ****************@

mus_weather_kyogre_2:
	.byte	KEYSH , mus_weather_kyogre_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 127
	.byte		LFOS  , 44
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 72*mus_weather_kyogre_mvl/mxv
	.byte	W24
mus_weather_kyogre_2_B1:
@ 001   ----------------------------------------
mus_weather_kyogre_2_001:
	.byte		N02   , Gn5 , v112
	.byte	W08
	.byte		        Gn5 , v072
	.byte	W08
	.byte		        Gn5 , v076
	.byte	W08
	.byte		        Gn5 , v112
	.byte	W08
	.byte		        Gn5 , v072
	.byte	W08
	.byte		        Gn5 , v076
	.byte	W08
	.byte		        Gn5 , v112
	.byte	W08
	.byte		        Gn5 , v072
	.byte	W08
	.byte		        Gn5 , v076
	.byte	W08
	.byte		        Gn5 , v112
	.byte	W08
	.byte		        Gn5 , v072
	.byte	W08
	.byte		        Gn5 , v076
	.byte	W08
	.byte	PEND
@ 002   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 003   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 004   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 005   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 006   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 007   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 008   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 009   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 012   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 013   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
@ 016   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_2_001
	.byte	GOTO
	 .word	mus_weather_kyogre_2_B1
mus_weather_kyogre_2_B2:
@ 017   ----------------------------------------
	.byte	FINE

@**************** Track 3 (Midi-Chn.3) ****************@

mus_weather_kyogre_3:
	.byte	KEYSH , mus_weather_kyogre_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 80
	.byte		LFOS  , 44
	.byte		XCMD  , xIECV , 18
	.byte		        xIECV , 16
	.byte		BENDR , 12
	.byte		PAN   , c_v-63
	.byte		VOL   , 67*mus_weather_kyogre_mvl/mxv
	.byte		N24   , An1 , v112
	.byte	W04
	.byte		VOL   , 75*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        85*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		MOD   , 8
	.byte		VOL   , 97*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        113*mus_weather_kyogre_mvl/mxv
	.byte	W08
mus_weather_kyogre_3_B1:
@ 001   ----------------------------------------
	.byte		VOL   , 96*mus_weather_kyogre_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		MOD   , 0
	.byte		N08   , Dn1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , Dn1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N12   , Ds1 , v112
	.byte	W16
	.byte		N04   
	.byte	W16
	.byte		N08   , Ds2 
	.byte	W08
	.byte		N04   , Dn1 
	.byte	W08
@ 002   ----------------------------------------
mus_weather_kyogre_3_002:
	.byte		N08   , Ds1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v056
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		        Dn1 , v112
	.byte	W08
	.byte		        Ds1 , v064
	.byte	W08
	.byte		        Ds1 , v100
	.byte	W08
	.byte		        Dn1 , v112
	.byte	W08
	.byte		N02   , Ds1 
	.byte	W04
	.byte		N02   
	.byte	W04
	.byte		N04   
	.byte	W08
	.byte		N08   , Dn1 
	.byte	W08
	.byte		N04   , Ds1 
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte	PEND
@ 003   ----------------------------------------
mus_weather_kyogre_3_003:
	.byte		N08   , Ds1 , v112
	.byte	W08
	.byte		N04   , En1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , Ds1 , v112
	.byte	W08
	.byte		N04   , En1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , En1 , v112
	.byte	W16
	.byte		N04   
	.byte	W16
	.byte		N08   , En2 
	.byte	W08
	.byte		N04   , En1 
	.byte	W08
	.byte	PEND
@ 004   ----------------------------------------
	.byte		N08   
	.byte	W08
	.byte		N04   , En1 , v056
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		        Ds1 , v112
	.byte	W08
	.byte		        En1 , v064
	.byte	W08
	.byte		        En1 , v100
	.byte	W08
	.byte		        Ds1 , v112
	.byte	W08
	.byte		N02   , En1 
	.byte	W04
	.byte		N02   
	.byte	W04
	.byte		N04   
	.byte	W08
	.byte		PAN   , c_v-62
	.byte		VOL   , 67*mus_weather_kyogre_mvl/mxv
	.byte		N24   
	.byte	W04
	.byte		VOL   , 75*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        85*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		MOD   , 13
	.byte		VOL   , 97*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        113*mus_weather_kyogre_mvl/mxv
	.byte	W08
@ 005   ----------------------------------------
	.byte		        96*mus_weather_kyogre_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		MOD   , 2
	.byte		N08   , Dn1 
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , Dn1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N12   , Ds1 , v112
	.byte	W16
	.byte		N04   
	.byte	W16
	.byte		N08   , Ds2 
	.byte	W08
	.byte		N04   , Dn1 
	.byte	W08
@ 006   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_002
@ 007   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_003
@ 008   ----------------------------------------
mus_weather_kyogre_3_008:
	.byte		N08   , En1 , v112
	.byte	W08
	.byte		N04   , En1 , v056
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		        Ds1 , v112
	.byte	W08
	.byte		        En1 , v064
	.byte	W08
	.byte		        En1 , v100
	.byte	W08
	.byte		        Ds1 , v112
	.byte	W08
	.byte		N02   , En1 
	.byte	W04
	.byte		N02   
	.byte	W04
	.byte		N04   
	.byte	W08
	.byte		PAN   , c_v-62
	.byte		VOL   , 67*mus_weather_kyogre_mvl/mxv
	.byte		N24   
	.byte	W04
	.byte		VOL   , 75*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        85*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        97*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        113*mus_weather_kyogre_mvl/mxv
	.byte	W08
	.byte	PEND
@ 009   ----------------------------------------
mus_weather_kyogre_3_009:
	.byte		VOL   , 96*mus_weather_kyogre_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N08   , Dn1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , Dn1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N12   , Ds1 , v112
	.byte	W16
	.byte		N04   
	.byte	W16
	.byte		N08   , Ds2 
	.byte	W08
	.byte		N04   , Dn1 
	.byte	W08
	.byte	PEND
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_002
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_003
@ 012   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_008
@ 013   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_009
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_002
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_003
@ 016   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_3_008
	.byte	GOTO
	 .word	mus_weather_kyogre_3_B1
mus_weather_kyogre_3_B2:
@ 017   ----------------------------------------
	.byte	FINE

@**************** Track 4 (Midi-Chn.4) ****************@

mus_weather_kyogre_4:
	.byte	KEYSH , mus_weather_kyogre_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 81
	.byte		LFOS  , 44
	.byte		XCMD  , xIECV , 18
	.byte		        xIECV , 16
	.byte		BENDR , 12
	.byte		VOL   , 64*mus_weather_kyogre_mvl/mxv
	.byte		PAN   , c_v+63
	.byte		N24   , En1 , v112
	.byte	W04
	.byte		VOL   , 67*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        75*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		MOD   , 8
	.byte		VOL   , 85*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        97*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        113*mus_weather_kyogre_mvl/mxv
	.byte	W04
mus_weather_kyogre_4_B1:
@ 001   ----------------------------------------
	.byte		PAN   , c_v+0
	.byte		VOL   , 16*mus_weather_kyogre_mvl/mxv
	.byte		MOD   , 0
	.byte		TIE   , Ds3 , v112
	.byte	W48
	.byte		VOL   , 21*mus_weather_kyogre_mvl/mxv
	.byte	W48
@ 002   ----------------------------------------
mus_weather_kyogre_4_002:
	.byte		VOL   , 32*mus_weather_kyogre_mvl/mxv
	.byte	W24
	.byte		        48*mus_weather_kyogre_mvl/mxv
	.byte	W24
	.byte		        64*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        81*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        96*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        116*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte	PEND
	.byte		EOT   , Ds3 
@ 003   ----------------------------------------
	.byte		VOL   , 64*mus_weather_kyogre_mvl/mxv
	.byte		TIE   , Ds3 , v112
	.byte	W48
	.byte		MOD   , 3
	.byte	W12
	.byte		VOL   , 58*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        50*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        43*mus_weather_kyogre_mvl/mxv
	.byte	W12
@ 004   ----------------------------------------
	.byte		        37*mus_weather_kyogre_mvl/mxv
	.byte		MOD   , 5
	.byte	W12
	.byte		VOL   , 31*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        24*mus_weather_kyogre_mvl/mxv
	.byte		MOD   , 9
	.byte	W12
	.byte		VOL   , 18*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        16*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        16*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		EOT   
	.byte		PAN   , c_v+63
	.byte		VOL   , 43*mus_weather_kyogre_mvl/mxv
	.byte		MOD   , 2
	.byte		N24   , Bn1 
	.byte	W04
	.byte		VOL   , 67*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        75*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		MOD   , 13
	.byte		VOL   , 85*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        97*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        113*mus_weather_kyogre_mvl/mxv
	.byte	W04
@ 005   ----------------------------------------
	.byte		PAN   , c_v+0
	.byte		VOL   , 16*mus_weather_kyogre_mvl/mxv
	.byte		MOD   , 2
	.byte		TIE   , Ds3 
	.byte	W48
	.byte		VOL   , 21*mus_weather_kyogre_mvl/mxv
	.byte	W48
@ 006   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_4_002
	.byte		EOT   , Ds3 
@ 007   ----------------------------------------
mus_weather_kyogre_4_007:
	.byte		VOL   , 64*mus_weather_kyogre_mvl/mxv
	.byte		TIE   , Ds3 , v112
	.byte	W60
	.byte		VOL   , 58*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        50*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        43*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte	PEND
@ 008   ----------------------------------------
mus_weather_kyogre_4_008:
	.byte		VOL   , 37*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        31*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        24*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        18*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        16*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte		        16*mus_weather_kyogre_mvl/mxv
	.byte	W12
	.byte	PEND
	.byte		EOT   , Ds3 
	.byte		PAN   , c_v+63
	.byte		VOL   , 43*mus_weather_kyogre_mvl/mxv
	.byte		N24   , Bn1 , v112
	.byte	W04
	.byte		VOL   , 67*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        75*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        85*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        97*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        113*mus_weather_kyogre_mvl/mxv
	.byte	W04
@ 009   ----------------------------------------
mus_weather_kyogre_4_009:
	.byte		PAN   , c_v+0
	.byte		VOL   , 16*mus_weather_kyogre_mvl/mxv
	.byte		TIE   , Ds3 , v112
	.byte	W48
	.byte		VOL   , 21*mus_weather_kyogre_mvl/mxv
	.byte	W48
	.byte	PEND
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_4_002
	.byte		EOT   , Ds3 
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_4_007
@ 012   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_4_008
	.byte		EOT   , Ds3 
	.byte		PAN   , c_v+63
	.byte		VOL   , 43*mus_weather_kyogre_mvl/mxv
	.byte		N24   , Bn1 , v112
	.byte	W04
	.byte		VOL   , 67*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        75*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        85*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        97*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        113*mus_weather_kyogre_mvl/mxv
	.byte	W04
@ 013   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_4_009
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_4_002
	.byte		EOT   , Ds3 
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_4_007
@ 016   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_4_008
	.byte		EOT   , Ds3 
	.byte		PAN   , c_v+63
	.byte		VOL   , 43*mus_weather_kyogre_mvl/mxv
	.byte		N24   , Bn1 , v112
	.byte	W04
	.byte		VOL   , 67*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        75*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        85*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        97*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte		        113*mus_weather_kyogre_mvl/mxv
	.byte	W04
	.byte	GOTO
	 .word	mus_weather_kyogre_4_B1
mus_weather_kyogre_4_B2:
@ 017   ----------------------------------------
	.byte	FINE

@**************** Track 5 (Midi-Chn.5) ****************@

mus_weather_kyogre_5:
	.byte	KEYSH , mus_weather_kyogre_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 0
	.byte		PAN   , c_v+0
	.byte		MOD   , 64
	.byte		VOL   , 127*mus_weather_kyogre_mvl/mxv
	.byte	W24
mus_weather_kyogre_5_B1:
@ 001   ----------------------------------------
mus_weather_kyogre_5_001:
	.byte		N04   , Cn1 , v127
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W24
	.byte		N04   
	.byte	W16
	.byte		N04   
	.byte	W24
	.byte		        Cn1 , v088
	.byte	W08
	.byte	PEND
@ 002   ----------------------------------------
mus_weather_kyogre_5_002:
	.byte		N04   , Cn1 , v127
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W24
	.byte		N04   
	.byte	W16
	.byte		N04   
	.byte	W16
	.byte		        Dn1 , v127
	.byte	W08
	.byte		        Cn1 , v088
	.byte	W08
	.byte	PEND
@ 003   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 004   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 005   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_001
@ 006   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 007   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 008   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 009   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_001
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 012   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 013   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_001
@ 014   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 015   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
@ 016   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_5_002
	.byte	GOTO
	 .word	mus_weather_kyogre_5_B1
mus_weather_kyogre_5_B2:
@ 017   ----------------------------------------
	.byte	FINE

@**************** Track 6 (Midi-Chn.6) ****************@

mus_weather_kyogre_6:
	.byte	KEYSH , mus_weather_kyogre_key+0
@ 000   ----------------------------------------
	.byte		VOICE , 82
	.byte		LFOS  , 44
	.byte		XCMD  , xIECV , 18
	.byte		        xIECV , 16
	.byte		BENDR , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 64*mus_weather_kyogre_mvl/mxv
	.byte		BEND  , c_v+1
	.byte		N24   , An1 , v112
	.byte	W12
	.byte		MOD   , 8
	.byte	W12
mus_weather_kyogre_6_B1:
@ 001   ----------------------------------------
	.byte		VOICE , 82
	.byte		MOD   , 0
	.byte		N08   , Dn1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , Dn1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N12   , Ds1 , v112
	.byte	W16
	.byte		N04   
	.byte	W16
	.byte		N08   , Ds2 
	.byte	W08
	.byte		N04   , Dn1 
	.byte	W08
@ 002   ----------------------------------------
mus_weather_kyogre_6_002:
	.byte		N08   , Ds1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v056
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		        Dn1 , v112
	.byte	W08
	.byte		        Ds1 , v064
	.byte	W08
	.byte		        Ds1 , v100
	.byte	W08
	.byte		        Dn1 , v112
	.byte	W08
	.byte		N02   , Ds1 
	.byte	W04
	.byte		N02   
	.byte	W04
	.byte		N04   
	.byte	W08
	.byte		N08   , Dn1 
	.byte	W08
	.byte		N04   , Ds1 
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte	PEND
@ 003   ----------------------------------------
mus_weather_kyogre_6_003:
	.byte		N08   , Ds1 , v112
	.byte	W08
	.byte		N04   , En1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , Ds1 , v112
	.byte	W08
	.byte		N04   , En1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , En1 , v112
	.byte	W16
	.byte		N04   
	.byte	W16
	.byte		N08   , En2 
	.byte	W08
	.byte		N04   , En1 
	.byte	W08
	.byte	PEND
@ 004   ----------------------------------------
	.byte		N08   
	.byte	W08
	.byte		N04   , En1 , v056
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		        Ds1 , v112
	.byte	W08
	.byte		        En1 , v064
	.byte	W08
	.byte		        En1 , v100
	.byte	W08
	.byte		        Ds1 , v112
	.byte	W08
	.byte		N02   , En1 
	.byte	W04
	.byte		N02   
	.byte	W04
	.byte		N04   
	.byte	W08
	.byte		N24   
	.byte	W12
	.byte		MOD   , 13
	.byte	W12
@ 005   ----------------------------------------
	.byte		VOICE , 84
	.byte		MOD   , 0
	.byte		BEND  , c_v+0
	.byte		N16   , As4 
	.byte	W16
	.byte		N04   , Cs5 
	.byte	W08
	.byte		        Cn5 
	.byte	W16
	.byte		        An4 
	.byte	W08
	.byte		        Ds4 
	.byte	W08
	.byte		        Dn4 
	.byte	W08
	.byte		        Fs5 
	.byte	W16
	.byte		        An4 
	.byte	W08
	.byte		        As4 
	.byte	W08
@ 006   ----------------------------------------
	.byte		N40   , Ds4 
	.byte	W12
	.byte		MOD   , 7
	.byte	W28
	.byte		        0
	.byte		N04   , Dn4 
	.byte	W04
	.byte		        Ds4 
	.byte	W04
	.byte		N24   , Fs4 
	.byte	W12
	.byte		MOD   , 7
	.byte	W12
	.byte		        0
	.byte		N24   , Gs4 
	.byte	W12
	.byte		MOD   , 7
	.byte	W12
@ 007   ----------------------------------------
	.byte		        0
	.byte		N08   , Bn4 
	.byte	W08
	.byte		N04   , As4 
	.byte	W08
	.byte		        Fs4 
	.byte	W08
	.byte		N08   , Gs4 
	.byte	W08
	.byte		N04   , Bn4 
	.byte	W08
	.byte		        As4 
	.byte	W08
	.byte		        Fs5 
	.byte	W08
	.byte		        Gs5 
	.byte	W16
	.byte		        Fs4 
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N04   
	.byte	W08
@ 008   ----------------------------------------
	.byte		N42   , Ds4 
	.byte	W12
	.byte		MOD   , 7
	.byte	W30
	.byte		        0
	.byte		N04   , Dn4 
	.byte	W04
	.byte		        Ds4 
	.byte	W02
	.byte		N24   , Fs4 
	.byte	W12
	.byte		MOD   , 7
	.byte	W12
	.byte		        0
	.byte		N24   , Gs4 
	.byte	W14
	.byte		MOD   , 7
	.byte	W10
@ 009   ----------------------------------------
	.byte		        0
	.byte		BEND  , c_v+2
	.byte		N08   , Dn1 
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N08   , Dn1 , v112
	.byte	W08
	.byte		N04   , Ds1 , v076
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N12   , Ds1 , v112
	.byte	W16
	.byte		N04   
	.byte	W16
	.byte		N08   , Ds2 
	.byte	W08
	.byte		N04   , Dn1 
	.byte	W08
@ 010   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_6_002
@ 011   ----------------------------------------
	.byte	PATT
	 .word	mus_weather_kyogre_6_003
@ 012   ----------------------------------------
	.byte		N08   , En1 , v112
	.byte	W08
	.byte		N04   , En1 , v056
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		        Ds1 , v112
	.byte	W08
	.byte		        En1 , v064
	.byte	W08
	.byte		        En1 , v100
	.byte	W08
	.byte		        Ds1 , v112
	.byte	W08
	.byte		N02   , En1 
	.byte	W04
	.byte		N02   
	.byte	W04
	.byte		N04   
	.byte	W08
	.byte		N24   
	.byte	W24
@ 013   ----------------------------------------
	.byte		VOICE , 83
	.byte		BEND  , c_v+0
	.byte		N16   , As3 
	.byte	W16
	.byte		N04   , Cs4 
	.byte	W08
	.byte		        Cn4 
	.byte	W16
	.byte		        An3 
	.byte	W08
	.byte		        Ds3 
	.byte	W08
	.byte		        Dn3 
	.byte	W08
	.byte		        Fs4 
	.byte	W16
	.byte		        An3 
	.byte	W08
	.byte		        As3 
	.byte	W08
@ 014   ----------------------------------------
	.byte		N40   , Ds3 
	.byte	W12
	.byte		MOD   , 7
	.byte	W28
	.byte		        0
	.byte		N04   , Dn3 
	.byte	W04
	.byte		        Ds3 
	.byte	W04
	.byte		N24   , Fs3 
	.byte	W12
	.byte		MOD   , 7
	.byte	W12
	.byte		        0
	.byte		N24   , Gs3 
	.byte	W12
	.byte		MOD   , 7
	.byte	W12
@ 015   ----------------------------------------
	.byte		        0
	.byte		N08   , Bn3 
	.byte	W08
	.byte		N04   , As3 
	.byte	W08
	.byte		        Fs3 
	.byte	W08
	.byte		N08   , Gs3 
	.byte	W08
	.byte		N04   , Bn3 
	.byte	W08
	.byte		        As3 
	.byte	W08
	.byte		        Fs4 
	.byte	W08
	.byte		        Gs4 
	.byte	W16
	.byte		        Fs3 
	.byte	W08
	.byte		N04   
	.byte	W08
	.byte		N04   
	.byte	W08
@ 016   ----------------------------------------
	.byte		N42   , Ds3 
	.byte	W12
	.byte		MOD   , 7
	.byte	W30
	.byte		        0
	.byte		N04   , Dn3 
	.byte	W04
	.byte		        Ds3 
	.byte	W02
	.byte		N24   , Fs3 
	.byte	W12
	.byte		MOD   , 7
	.byte	W12
	.byte		        0
	.byte		N24   , Gs3 
	.byte	W14
	.byte		MOD   , 7
	.byte	W10
	.byte	GOTO
	 .word	mus_weather_kyogre_6_B1
mus_weather_kyogre_6_B2:
@ 017   ----------------------------------------
	.byte		MOD   , 0
	.byte	FINE

@******************************************************@
	.align	2

mus_weather_kyogre:
	.byte	6	@ NumTrks
	.byte	0	@ NumBlks
	.byte	mus_weather_kyogre_pri	@ Priority
	.byte	mus_weather_kyogre_rev	@ Reverb.

	.word	mus_weather_kyogre_grp

	.word	mus_weather_kyogre_1
	.word	mus_weather_kyogre_2
	.word	mus_weather_kyogre_3
	.word	mus_weather_kyogre_4
	.word	mus_weather_kyogre_5
	.word	mus_weather_kyogre_6

	.end
