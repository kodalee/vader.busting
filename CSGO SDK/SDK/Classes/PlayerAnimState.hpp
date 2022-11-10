#pragma once
#include "../sdk.hpp"

class C_CSPlayer;
class C_WeaponCSBaseGun;

struct animstate_pose_param_cache_t {
	bool valid = false;
	int index = -1;
	const char* name;
	void SetValue(C_CSPlayer* player, float flValue);
};

enum animstate_pose_param_idx_t {
	PLAYER_POSE_PARAM_FIRST = 0,
	PLAYER_POSE_PARAM_LEAN_YAW = PLAYER_POSE_PARAM_FIRST,
	PLAYER_POSE_PARAM_SPEED,
	PLAYER_POSE_PARAM_LADDER_SPEED,
	PLAYER_POSE_PARAM_LADDER_YAW,
	PLAYER_POSE_PARAM_MOVE_YAW,
	PLAYER_POSE_PARAM_RUN,
	PLAYER_POSE_PARAM_BODY_YAW,
	PLAYER_POSE_PARAM_BODY_PITCH,
	PLAYER_POSE_PARAM_DEATH_YAW,
	PLAYER_POSE_PARAM_STAND,
	PLAYER_POSE_PARAM_JUMP_FALL,
	PLAYER_POSE_PARAM_AIM_BLEND_STAND_IDLE,
	PLAYER_POSE_PARAM_AIM_BLEND_CROUCH_IDLE,
	PLAYER_POSE_PARAM_STRAFE_DIR,
	PLAYER_POSE_PARAM_AIM_BLEND_STAND_WALK,
	PLAYER_POSE_PARAM_AIM_BLEND_STAND_RUN,
	PLAYER_POSE_PARAM_AIM_BLEND_CROUCH_WALK,
	PLAYER_POSE_PARAM_MOVE_BLEND_WALK,
	PLAYER_POSE_PARAM_MOVE_BLEND_RUN,
	PLAYER_POSE_PARAM_MOVE_BLEND_CROUCH_WALK,
	//PLAYER_POSE_PARAM_STRAFE_CROSS,
	PLAYER_POSE_PARAM_COUNT,
};

class CCSGOPlayerAnimState {
public:
	int* m_layer_order_preset = nullptr;
	bool					m_first_run_since_init = false;

	bool					m_first_foot_plant_since_init = false;
	int						m_last_update_tick = 0;
	float					m_eye_position_smooth_lerp = 0.0f;

	float					m_strafe_change_weight_smooth_fall_off = 0.0f;

	float					m_stand_walk_duration_state_has_been_valid = 0.0f;
	float					m_stand_walk_duration_state_has_been_invalid = 0.0f;
	float					m_stand_walk_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float					m_stand_walk_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float					m_stand_walk_blend_value = 0.0f;

	float					m_stand_run_duration_state_has_been_valid = 0.0f;
	float					m_stand_run_duration_state_has_been_invalid = 0.0f;
	float					m_stand_run_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float					m_stand_run_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float					m_stand_run_blend_value = 0.0f;

	float					m_crouch_walk_duration_state_has_been_valid = 0.0f;
	float					m_crouch_walk_duration_state_has_been_invalid = 0.0f;
	float					m_crouch_walk_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float					m_crouch_walk_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float					m_crouch_walk_blend_value = 0.0f;

	int						m_cached_model_index = 0;

	float					m_step_height_left = 0.0f;
	float					m_step_height_right = 0.0f;

	C_WeaponCSBaseGun* m_weapon_last_bone_setup = nullptr;

	C_CSPlayer* m_player = nullptr;//0x0060 
	C_WeaponCSBaseGun* m_weapon = nullptr;//0x0064
	C_WeaponCSBaseGun* m_weapon_last = nullptr;//0x0068

	float					m_last_update_time = 0.0f;//0x006C	
	int						m_last_update_frame = 0;//0x0070 
	float					m_last_update_increment = 0.0f;//0x0074 

	float					m_eye_yaw = 0.0f; //0x0078 
	float					m_eye_pitch = 0.0f; //0x007C 
	float					m_abs_yaw = 0.0f; //0x0080 : absRotation : goalFeetYaw : footYaw
	float					m_abs_yaw_last = 0.0f; //0x0084 : absRotationOld : currentFeetYaw : footYawLast
	float					m_move_yaw = 0.0f; //0x0088 : currentTorsoYaw
	float					m_move_yaw_ideal = 0.0f; //0x008C : unknwownVelocityLean
	float					m_move_yaw_current_to_ideal = 0.0f; //0x0090 : leanAmount
	float					m_time_to_align_lower_body;

	float					m_primary_cycle = 0.0f; // m_flFeetCycle //0x0098
	float					m_move_weight = 0.0f; // m_flFeetYawRate //0x009C 

	float					m_move_weight_smoothed = 0.0f;
	float					m_anim_duck_amount = 0.0f; //0x00A4
	float					m_duck_additional = 0.0f; //0x00A8 : landingDuckAdditiveSomething
	float					m_recrouch_weight = 0.0f;

	Vector					m_position_current = Vector(0, 0, 0); //0x00B0 : origin
	Vector					m_position_last = Vector(0, 0, 0); //0x00BC : originLast

	Vector					m_velocity = Vector(0, 0, 0); //0x00C8
	Vector					m_velocity_normalized = Vector(0, 0, 0); // 
	Vector					m_velocity_normalized_non_zero = Vector(0, 0, 0); //0x00E0
	float					m_velocity_length_xy = 0.0f; //0x00EC
	float					m_velocity_length_z = 0.0f; //0x00F0

	float					m_speed_as_portion_of_run_top_speed = 0.0f; //0x00F4
	float					m_speed_as_portion_of_walk_top_speed = 0.0f; //0x00F8 
	float					m_speed_as_portion_of_crouch_top_speed = 0.0f; //0x00FC

	float					m_duration_moving = 0.0f; //0x0100 // timeSinceStartedMoving
	float					m_duration_still = 0.0f; //0x0104 // timeSinceStoppedMoving

	bool					m_on_ground = false; //0x0108 

	bool					m_landing = false; //0x0109 : hitGroundAnimation
	float					m_jump_to_fall = 0.0f;
	float					m_duration_in_air = 0.0f; //0x0110
	float					m_left_ground_height = 0.0f; //0x0114 
	float					m_land_anim_multiplier = 0.0f; //0x0118 

	float					m_walk_run_transition = 0.0f; //0x011C

	bool					m_landed_on_ground_this_frame = false;
	bool					m_left_the_ground_this_frame = false;
	float					m_in_air_smooth_value = 0.0f;

	bool					m_on_ladder = false; //0x0124
	float					m_ladder_weight = 0.0f; //0x0128
	float					m_ladder_speed = 0.0f;

	bool					m_walk_to_run_transition_state = false;

	bool					m_defuse_started = false;
	bool					m_plant_anim_started = false;
	bool					m_twitch_anim_started = false;
	bool					m_adjust_started = false;

	char					m_activity_modifiers_server[20] = {};
	//CUtlVector<CUtlSymbol>	m_activity_modifiers;

	float					m_next_twitch_time = 0.0f;

	float					m_time_of_last_known_injury = 0.0f;

	float					m_last_velocity_test_time = 0.0f;
	Vector					m_velocity_last = Vector(0, 0, 0);
	Vector					m_target_acceleration = Vector(0, 0, 0);
	Vector					m_acceleration = Vector(0, 0, 0);
	float					m_acceleration_weight = 0.0f;

	float					m_aim_matrix_transition = 0.0f;
	float					m_aim_matrix_transition_delay = 0.0f;

	bool					m_flashed = false;

	float					m_strafe_change_weight = 0.0f;
	float					m_strafe_change_target_weight = 0.0f;
	float					m_strafe_change_cycle = 0.0f;
	int						m_strafe_sequence = 0;
	bool					m_strafe_changing = false;
	float					m_duration_strafing = 0.0f;

	float					m_foot_lerp = 0.0f;

	bool					m_feet_crossed = false;

	bool					m_player_is_accelerating = false;

	animstate_pose_param_cache_t m_pose_param_mappings[PLAYER_POSE_PARAM_COUNT] = {};

	float					m_duration_move_weight_is_too_high = 0.0f;
	float					m_static_approach_speed = 0.0f;

	int						m_previous_move_state = 0;
	float					m_stutter_step = 0.0f;

	float					m_action_weight_bias_remainder = 0.0f;

	Vector m_foot_left_pos_anim = Vector(0, 0, 0);
	Vector m_foot_left_pos_anim_last = Vector(0, 0, 0);
	Vector m_foot_left_pos_plant = Vector(0, 0, 0);
	Vector m_foot_left_plant_vel = Vector(0, 0, 0);
	float m_foot_left_lock_amount = 0.0f;
	float m_foot_left_last_plant_time = 0.0f;

	Vector m_foot_right_pos_anim = Vector(0, 0, 0);
	Vector m_foot_right_pos_anim_last = Vector(0, 0, 0);
	Vector m_foot_right_pos_plant = Vector(0, 0, 0);
	Vector m_foot_right_plant_vel = Vector(0, 0, 0);
	float m_foot_right_lock_amount = 0.0f;
	float m_foot_right_last_plant_time = 0.0f;

	float					m_camera_smooth_height = 0.0f;
	bool					m_smooth_height_valid = false;
	float					m_last_time_velocity_over_ten = 0.0f; // 0x032C

	float					m_aim_yaw_min = 0.0f;//0x0330
	float					m_aim_yaw_max = 0.0f;//0x0334
	float					m_aim_pitch_min = 0.0f;
	float					m_aim_pitch_max = 0.0f;

	int						m_animstate_model_version = 0;

	void Reset();
	void Update(QAngle angles);
	const char* GetWeaponPrefix();
	static void ModifyEyePosition(CCSGOPlayerAnimState* pState, Vector* pos);
};//Size=0x344
