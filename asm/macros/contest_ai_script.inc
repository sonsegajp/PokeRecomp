

	.macro score score
	.byte 0x00
	.byte \score
	.endm



	.macro get_turn
	.byte 0x01
	.endm

	.macro if_turn_less_than param, addr
	.byte 0x02
	.byte \param
	.4byte \addr
	.endm

	.macro if_turn_more_than param, addr
	.byte 0x03
	.byte \param
	.4byte \addr
	.endm

	.macro if_turn_eq param, addr
	.byte 0x04
	.byte \param
	.4byte \addr
	.endm

	.macro if_turn_not_eq param, addr
	.byte 0x05
	.byte \param
	.4byte \addr
	.endm



	.macro get_excitement
	.byte 0x06
	.endm

	.macro if_excitement_less_than param, addr
	.byte 0x07
	.byte \param
	.4byte \addr
	.endm

	.macro if_excitement_more_than param, addr
	.byte 0x08
	.byte \param
	.4byte \addr
	.endm

	.macro if_excitement_eq param, addr
	.byte 0x09
	.byte \param
	.4byte \addr
	.endm

	.macro if_excitement_not_eq param, addr
	.byte 0x0A
	.byte \param
	.4byte \addr
	.endm



	.macro get_user_order
	.byte 0x0B
	.endm

	.macro if_user_order_less_than param addr
	.byte 0x0C
	.byte \param
	.4byte \addr
	.endm

	.macro if_user_order_more_than param addr
	.byte 0x0D
	.byte \param
	.4byte \addr
	.endm

	.macro if_user_order_eq param addr
	.byte 0x0E
	.byte \param
	.4byte \addr
	.endm

	.macro if_user_order_not_eq param addr
	.byte 0x0F
	.byte \param
	.4byte \addr
	.endm



	.macro get_user_condition
	.byte 0x10
	.endm

	.macro if_user_condition_less_than param, addr
	.byte 0x11
	.byte \param
	.4byte \addr
	.endm

	.macro if_user_condition_more_than param, addr
	.byte 0x12
	.byte \param
	.4byte \addr
	.endm

	.macro if_user_condition_eq param, addr
	.byte 0x13
	.byte \param
	.4byte \addr
	.endm

	.macro if_user_condition_not_eq param, addr
	.byte 0x14
	.byte \param
	.4byte \addr
	.endm














	.macro get_contest_type
	.byte 0x1F
	.endm

	.macro if_contest_type_eq param, addr
	.byte 0x20
	.byte \param
	.4byte \addr
	.endm

	.macro if_contest_type_not_eq param, addr
	.byte 0x21
	.byte \param
	.4byte \addr
	.endm



	.macro get_move_excitement
	.byte 0x22
	.endm

	.macro if_move_excitement_less_than param, addr
	.byte 0x23
	.byte \param
	.4byte \addr
	.endm

	.macro if_move_excitement_more_than param, addr
	.byte 0x24
	.byte \param
	.4byte \addr
	.endm

	.macro if_move_excitement_eq param, addr
	.byte 0x25
	.byte \param
	.4byte \addr
	.endm

	.macro if_move_excitement_not_eq param, addr
	.byte 0x26
	.byte \param
	.4byte \addr
	.endm



	.macro get_effect
	.byte 0x27
	.endm

	.macro if_effect_eq param, addr
	.byte 0x28
	.byte \param
	.4byte \addr
	.endm

	.macro if_effect_not_eq param, addr
	.byte 0x29
	.byte \param
	.4byte \addr
	.endm



	.macro get_effect_type
	.byte 0x2A
	.endm

	.macro if_effect_type_eq param, addr
	.byte 0x2B
	.byte \param
	.4byte \addr
	.endm

	.macro if_effect_type_not_eq param, addr
	.byte 0x2C
	.byte \param
	.4byte \addr
	.endm



	.macro check_most_appealing_move
	.byte 0x2D
	.endm

	.macro if_most_appealing_move addr
	.byte 0x2E
	.4byte \addr
	.endm
















	.macro get_move_used_count
	.byte 0x3B
	.endm

	.macro if_move_used_count_less_than param, addr
	.byte 0x3C
	.byte \param
	.4byte \addr
	.endm

	.macro if_move_used_count_more_than param, addr
	.byte 0x3D
	.byte \param
	.4byte \addr
	.endm

	.macro if_move_used_count_eq param, addr
	.byte 0x3E
	.byte \param
	.4byte \addr
	.endm

	.macro if_move_used_count_not_eq param, addr
	.byte 0x3F
	.byte \param
	.4byte \addr
	.endm



	.macro check_combo_starter
	.byte 0x40
	.endm

	.macro if_combo_starter addr
	.byte 0x41
	.4byte \addr
	.endm

	.macro if_not_combo_starter addr
	.byte 0x42
	.4byte \addr
	.endm



	.macro check_combo_finisher
	.byte 0x43
	.endm

	.macro if_combo_finisher addr
	.byte 0x44
	.4byte \addr
	.endm

	.macro if_not_combo_finisher addr
	.byte 0x45
	.4byte \addr
	.endm



	.macro check_would_finish_combo
	.byte 0x46
	.endm

	.macro if_would_finish_combo addr
	.byte 0x47
	.4byte \addr
	.endm

	.macro if_would_not_finish_combo addr
	.byte 0x48
	.4byte \addr
	.endm



	.macro get_condition mon
	.byte 0x49
	.byte \mon
	.endm

	.macro if_condition_less_than mon, value, addr
	.byte 0x4A
	.byte \mon
	.byte \value
	.4byte \addr
	.endm

	.macro if_condition_more_than mon, value, addr
	.byte 0x4B
	.byte \mon
	.byte \value
	.4byte \addr
	.endm

	.macro if_condition_eq mon, value, addr
	.byte 0x4C
	.byte \mon
	.byte \value
	.4byte \addr
	.endm

	.macro if_condition_not_eq mon, value, addr
	.byte 0x4D
	.byte \mon
	.byte \value
	.4byte \addr
	.endm





	.macro get_used_combo_starter mon
	.byte 0x4E
	.byte \mon
	.endm

	.macro if_used_combo_starter_less_than mon, value, addr
	.byte 0x4F
	.byte \mon
	.byte \value
	.4byte \addr
	.endm

	.macro if_used_combo_starter_more_than mon, value, addr
	.byte 0x50
	.byte \mon
	.byte \value
	.4byte \addr
	.endm


	.macro if_used_combo_starter_eq mon, value, addr
	.byte 0x51
	.byte \mon
	.byte \value
	.4byte \addr
	.endm

	.macro if_used_combo_starter_not_eq mon, value, addr
	.byte 0x52
	.byte \mon
	.byte \value
	.4byte \addr
	.endm



	.macro check_can_participate mon
	.byte 0x53
	.byte \mon
	.endm

	.macro if_can_participate mon, addr
	.byte 0x54
	.byte \mon
	.4byte \addr
	.endm

	.macro if_cannot_participate mon, addr
	.byte 0x55
	.byte \mon
	.4byte \addr
	.endm




	.macro contest_58 param addr
	.byte 0x58
	.byte \param
	.4byte \addr
	.endm






































	.macro if_random param addr
	.byte 0x7D
	.byte \param
	.4byte \addr
	.endm



	.macro jump addr
	.byte 0x7F
	.4byte \addr
	.endm

	.macro call addr
	.byte 0x80
	.4byte \addr
	.endm

	.macro end
	.byte 0x81
	.endm

	.macro check_user_has_exciting_move
	.byte 0x82
	.endm

	.macro if_user_has_exciting_move addr
	.byte 0x83
	.4byte \addr
	.endm

	.macro if_user_doesnt_have_exciting_move addr
	.byte 0x84
	.4byte \addr
	.endm




	.macro if_effect_in_user_moveset param addr
	.byte 0x87
	.2byte \param
	.4byte \addr
	.endm
