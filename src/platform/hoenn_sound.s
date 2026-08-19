# Hoenn sound data — Ruby's m4a voice groups, instruments, and songs
# Compiled into FireRed's binary for native GBA sound

	.section .rodata

	.include "../pokeruby/include/macros/m4a.inc"
	.include "../pokeruby/include/macros/music_voice.inc"

# Voice groups (instrument definitions)
	.include "../pokeruby/sound/voice_groups.inc"

# Keysplit tables
	.include "../pokeruby/sound/keysplit_tables.inc"

# Programmable wave data
	.include "../pokeruby/sound/programmable_wave_data.inc"

# Direct sound data (PCM samples)
	.include "../pokeruby/sound/direct_sound_data.inc"

# Song data — include individual songs we need
	.include "../pokeruby/include/MPlayDef.s"
	.include "../pokeruby/sound/songs/midi/mus_littleroot.s"
	.include "../pokeruby/sound/songs/midi/mus_oldale.s"
	.include "../pokeruby/sound/songs/midi/mus_petalburg.s"
	.include "../pokeruby/sound/songs/midi/mus_rustboro.s"
	.include "../pokeruby/sound/songs/midi/mus_dewford.s"
	.include "../pokeruby/sound/songs/midi/mus_slateport.s"
	.include "../pokeruby/sound/songs/midi/mus_cycling.s"
	.include "../pokeruby/sound/songs/midi/mus_verdanturf.s"
	.include "../pokeruby/sound/songs/midi/mus_fallarbor.s"
	.include "../pokeruby/sound/songs/midi/mus_route113.s"
	.include "../pokeruby/sound/songs/midi/mus_fortree.s"
	.include "../pokeruby/sound/songs/midi/mus_lilycove.s"
	.include "../pokeruby/sound/songs/midi/mus_route119.s"
	.include "../pokeruby/sound/songs/midi/mus_sootopolis.s"
	.include "../pokeruby/sound/songs/midi/mus_ever_grande.s"
	.include "../pokeruby/sound/songs/midi/mus_route101.s"
	.include "../pokeruby/sound/songs/midi/mus_route104.s"
	.include "../pokeruby/sound/songs/midi/mus_route110.s"
	.include "../pokeruby/sound/songs/midi/mus_route120.s"
	.include "../pokeruby/sound/songs/midi/mus_route122.s"
	.include "../pokeruby/sound/songs/midi/mus_surf.s"
	.include "../pokeruby/sound/songs/midi/mus_mt_chimney.s"
	.include "../pokeruby/sound/songs/midi/mus_gym.s"
	.include "../pokeruby/sound/songs/midi/mus_poke_center.s"
	.include "../pokeruby/sound/songs/midi/mus_poke_mart.s"
	.include "../pokeruby/sound/songs/midi/mus_vs_wild.s"
	.include "../pokeruby/sound/songs/midi/mus_vs_trainer.s"
	.include "../pokeruby/sound/songs/midi/mus_vs_gym_leader.s"
	.include "../pokeruby/sound/songs/midi/mus_vs_champion.s"
	.include "../pokeruby/sound/songs/midi/mus_vs_elite_four.s"
	.include "../pokeruby/sound/songs/midi/mus_vs_aqua_magma.s"
	.include "../pokeruby/sound/songs/midi/mus_vs_rival.s"
	.include "../pokeruby/sound/songs/midi/mus_victory_trainer.s"
	.include "../pokeruby/sound/songs/midi/mus_victory_aqua_magma.s"

	.align 2
