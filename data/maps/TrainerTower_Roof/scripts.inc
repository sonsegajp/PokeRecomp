TrainerTower_Roof_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, TrainerTower_OnResume
	map_script MAP_SCRIPT_ON_FRAME_TABLE, TrainerTower_OnFrame
	.byte 0



TrainerTower_Roof_EventScript_Owner::
	call TrainerTower_EventScript_SpeakToOwner
	end
