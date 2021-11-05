#include "Resolver.hpp"
#include "../../SDK/CVariables.hpp"
#include "../Visuals/CChams.hpp"
#include "../Rage/AntiAim.hpp"
#include "../Rage/Ragebot.hpp"
#include "../Rage/Autowall.h"
#include "../Visuals/EventLogger.hpp"

static float NextLBYUpdate[65];
static float Add[65];

namespace Engine {
	// well alpha took supremacy resolver, but supremacy resolver is actual dogshit
	// so this is going to be a hard test for viopastins.

	CResolver g_Resolver;
	CResolverData g_ResolverData[65];

	void CResolver::on_lby_proxy(C_CSPlayer* entity, float* LowerBodyYaw)
	{
		float oldBodyYaw; // xmm4_4
		float nextPredictedSimtime; // xmm3_4
		float nextBodyUpdate = 0.f; // xmm3_4

		auto anim_data = AnimationSystem::Get()->GetAnimationData(entity->m_entIndex);
		if (!anim_data)
			return;

		if (anim_data->m_AnimationRecord.empty())
			return;

		auto anim_record = &anim_data->m_AnimationRecord.back();
		if (!anim_record)
			return;

		oldBodyYaw = anim_record->m_flLowerBodyYawTarget;
		if (oldBodyYaw != *LowerBodyYaw)
		{
			if (entity != C_CSPlayer::GetLocalPlayer() && entity->m_fFlags() & FL_ONGROUND)
			{
				nextPredictedSimtime = entity->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
				float vel = entity->m_vecVelocity().Length2D();
				if (vel > 0.1f && !Engine::g_ResolverData[entity->EntIndex()].fakewalking)
					nextBodyUpdate = nextPredictedSimtime + 0.22f;
				else /*if ( PlayerRecord->nextBodyUpdate <= nextPredictedSimtime )*/
					nextBodyUpdate = nextPredictedSimtime + 1.1f;

				if (nextBodyUpdate != 0.f)
					Engine::g_ResolverData[entity->EntIndex()].m_flNextBodyUpdate = nextBodyUpdate;
			}

			Engine::g_ResolverData[entity->EntIndex()].m_flLastLowerBodyYawTargetUpdateTime = entity->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
			Engine::g_ResolverData[entity->EntIndex()].m_flOldLowerBodyYawTarget = oldBodyYaw;
			Engine::g_ResolverData[entity->EntIndex()].m_flLowerBodyYawTarget = Math::AngleNormalize(*LowerBodyYaw);

			//util::print_dev_console( true, Color::Green(), "update time = %f\n", PlayerRecord->m_flLastLowerBodyYawTargetUpdateTime );
		}

		if (entity->m_vecVelocity().Length2D() > 0.1f && !(Engine::g_ResolverData[entity->EntIndex()].fakewalking)/* || entity->m_fFlags() & FL_DUCKING && entity->m_vecVelocity().Length2D() > 20.f*/)
		{
			Engine::g_ResolverData[entity->EntIndex()].m_sMoveData.m_flLowerBodyYawTarget = Math::AngleNormalize(*LowerBodyYaw);
			Engine::g_ResolverData[entity->EntIndex()].m_sMoveData.m_flLastMovingLowerBodyYawTargetTime = entity->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
		}
	}

	/*
	void CResolver::collect_wall_detect(const ClientFrameStage_t stage)
	{
		if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			return;

		if (!C_CSPlayer::GetLocalPlayer())
			return;

		auto g_pLocalPlayer = C_CSPlayer::GetLocalPlayer();

		last_eye_positions.insert(last_eye_positions.begin(), g_pLocalPlayer->m_vecOrigin() + g_pLocalPlayer->m_vecViewOffset());
		if (last_eye_positions.size() > 128)
			last_eye_positions.pop_back();

		auto nci = Interfaces::m_pEngine->GetNetChannelInfo();
		if (!nci)
			return;


		const int latency_ticks = TIME_TO_TICKS(nci->GetLatency(FLOW_OUTGOING));
		auto latency_based_eye_pos = last_eye_positions.size() <= latency_ticks ? last_eye_positions.back() : last_eye_positions[latency_ticks];

		for (auto i = 1; i < Interfaces::m_pGlobalVars->maxClients; i++)
		{
			//auto& log = player_log::get().get_log(i);
			C_CSPlayer* player = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(i);

			if (!player || player == g_pLocalPlayer)
			{
				continue;
			}

			if (player->IsTeammate(g_pLocalPlayer))
			{
				continue;
			}

			if (!player->m_iHealth() > 0)
			{
				continue;
			}

			if (player->IsDormant())
			{
				continue;
			}

			if (player->m_vecVelocity().Length2D() > 0.1f)
			{
				continue;
			}

			//if (!log.record.empty() && player->get_simtime() - log.record[0].m_sim_time == 0)
			//	continue;

			//if (log.m_vecLastNonDormantOrig != player->get_origin() && g_pLocalPlayer->get_alive())
			//{
			//	log.m_iMode = RMODE_WALL;
			//}

			//else if (player->get_simtime() - log.m_flLastLowerBodyYawTargetUpdateTime > 1.35f && log.m_vecLastNonDormantOrig == player->get_origin() && log.m_iMode == RMODE_MOVING)
			//{
			//	if (player->get_simtime() - log.m_flLastLowerBodyYawTargetUpdateTime > 1.65f)
			//	{
			//		log.m_iMode = RMODE_WALL;
			//	}
			//}
			//else {
			//	log.m_iMode = RMODE_NORMAL;
			//}

			if (m_iMode == 1)
			{
				const auto at_target_angle = Math::CalcAngle(player->m_vecOrigin(), last_eye);

				//Vector left_dir, right_dir, back_dir;
				//math::get().angle_vectors(Vector(0.f, at_target_angle.y - 90.f, 0.f), &left_dir);
				//math::get().angle_vectors(Vector(0.f, at_target_angle.y + 90.f, 0.f), &right_dir);
				//math::get().angle_vectors(Vector(0.f, at_target_angle.y + 180.f, 0.f), &back_dir);

				const auto eye_pos = player->GetEyePosition();
				//auto left_eye_pos = eye_pos + (left_dir * 16.f);
				//auto right_eye_pos = eye_pos + (right_dir * 16.f);
				//auto back_eye_pos = eye_pos + (back_dir * 16.f);

				const float height = 64;

				Vector direction_1, direction_2, direction_3;
				Math::AngleVectors(QAngle(0.f, Math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y - 90.f, 0.f), &direction_1);
				Math::AngleVectors(QAngle(0.f, Math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y + 90.f, 0.f), &direction_2);
				Math::AngleVectors(QAngle(0.f, Math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y + 180.f, 0.f), &direction_3);

				const auto left_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_1 * 16.f);
				const auto right_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_2 * 16.f);
				const auto back_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_3 * 16.f);

				//log.anti_freestanding_record.left_damage = penetration::get().get_damage(latency_based_eye_pos,
				anti_freestanding_record.left_damage = penetration::scale(player, left_damage[i], 1.f, HITGROUP_CHEST);

				//anti_freestanding_record.left_damage = penetration::scale(player, &left_damage[i],
				//	HITGROUP_CHEST);
				//data->anti_freestanding_record.right_damage = FEATURES::RAGEBOT::autowall.CalculateDamage(latency_based_eye_pos,
				//	right_eye_pos, local_player, entity, 1).damage;
				anti_freestanding_record.right_damage = penetration::scale(player, right_damage[i], 1.f, HITGROUP_CHEST);
				//penetration::get().get_damage(g_cl.m_local, player, right_eye_pos, &right_damage[i],
				//get_big_fucking_gun(), &latency_based_eye_pos);
			//BACKWARDS
				anti_freestanding_record.back_damage = penetration::scale(player, back_damage[i], 1.f, HITGROUP_CHEST);

				Ray ray;
				CGameTrace trace;
				CTraceFilterWorldOnly filter;

				Ray first_ray(left_eye_pos, latency_based_eye_pos);
				g_csgo.m_engine_trace->TraceRay(first_ray, MASK_ALL, &filter, &trace);
				anti_freestanding_record.left_fraction = trace.m_fraction;

				Ray second_ray(right_eye_pos, latency_based_eye_pos);
				g_csgo.m_engine_trace->TraceRay(second_ray, MASK_ALL, &filter, &trace);
				anti_freestanding_record.right_fraction = trace.m_fraction;

				Ray third_ray(back_eye_pos, latency_based_eye_pos);
				g_csgo.m_engine_trace->TraceRay(third_ray, MASK_ALL, &filter, &trace);
				anti_freestanding_record.back_fraction = trace.m_fraction;


				//penetration::get().get_damage(g_pLocalPlayer, player, left_eye_pos, &left_damage[i], get_big_fucking_gun(), &last_eye);
				//penetration::get().get_damage(g_pLocalPlayer, player, right_eye_pos, &right_damage[i], get_big_fucking_gun(), &last_eye);
				//penetration::get().get_damage(g_pLocalPlayer, player, back_eye_pos, &back_damage[i], get_big_fucking_gun(), &last_eye);
			}
		}
	}

	bool CResolver::AntiFreestanding(C_CSPlayer* entity, float& yaw)
	{

		const auto freestanding_record = anti_freestanding_record;

		//g_pEntitiyList->GetClientEntity(g_pEngine->GetLocalPlayer())

		auto local_player = C_CSPlayer::GetLocalPlayer();
		if (!local_player)
			return false;

		const float at_target_yaw = Math::CalcAngle(local_player->m_vecOrigin(), entity->m_vecOrigin()).y;

		if (freestanding_record.left_damage >= 20 && freestanding_record.right_damage >= 20)
			yaw = at_target_yaw + 180.f;

		auto set = false;

		if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0)
		{
			if (freestanding_record.right_fraction < freestanding_record.left_fraction) {
				set = true;
				yaw = at_target_yaw + 125.f;
			}
			else if (freestanding_record.right_fraction > freestanding_record.left_fraction) {
				set = true;
				yaw = at_target_yaw - 73.f;
			}
			else {
				yaw = at_target_yaw + 180.f;
			}
		}
		else
		{
			if (freestanding_record.left_damage > freestanding_record.right_damage) {
				yaw = at_target_yaw + 130.f;
				set = true;
			}
			else
				yaw = at_target_yaw + 180.f;
		}

		return true;
	}
	*/

	void CResolver::FindBestAngle(C_CSPlayer* player) {
		auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal)
			return;

		// constants.
		constexpr float STEP{ 4.f };
		constexpr float RANGE{ 32.f };

		// best target.
		struct AutoTarget_t { float fov; C_CSPlayer* player; };
		AutoTarget_t target{ 180.f + 1.f, nullptr };

		if (!player || player->IsDead())
			return;

		if (player->IsDormant())
			return;

		bool is_team = player->IsTeammate(player);
		if (is_team)
			return;

		auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!lag_data.IsValid())
			return;

		auto AngleDistance = [&](QAngle& angles, const Vector& start, const Vector& end) -> float {
			auto direction = end - start;
			auto aimAngles = direction.ToEulerAngles();
			auto delta = aimAngles - angles;
			delta.Normalize();

			return sqrtf(delta.x * delta.x + delta.y * delta.y);
		};

		float fov = AngleDistance(g_Vars.globals.m_pCmd->viewangles, g_Vars.globals.m_vecFixedEyePosition, player->WorldSpaceCenter());

		if (fov < target.fov) {
			target.fov = fov;
			target.player = player;
		}

		// get best origin based on target.
		auto m_vecOrigin = player->m_vecOrigin();

		// get predicted away angle for the player.
		Vector angAway;
		Math::VectorAngles(pLocal->m_vecOrigin() - m_vecOrigin, angAway);

		//if( !target.player ) {
		//	// set angle to backwards.
		//	g_ResolverData[ player->EntIndex( ) ].m_flBestYaw = angAway.y + 180.f;
		//	g_ResolverData[ player->EntIndex( ) ].m_flBestDistance = -1.f;
		//	g_ResolverData[ player->EntIndex( ) ].m_bCollectedFreestandData = false;
		//	return;
		//}

		// construct vector of angles to test.
		std::vector< AdaptiveAngle > angles{ };
		angles.emplace_back(angAway.y - 180.f);
		angles.emplace_back(angAway.y - 90.f);
		angles.emplace_back(angAway.y + 90.f);

		// start the trace at the enemy shoot pos.
		Vector start = g_Vars.globals.m_vecFixedEyePosition;

		Vector end = target.player->GetEyePosition();

		// see if we got any valid result.
		// if this is false the path was not obstructed with anything.
		bool valid{ false };

		// iterate vector of angles.
		for (auto it = angles.begin(); it != angles.end(); ++it) {

			// compute the 'rough' estimation of where our head will be.
			Vector end{ end.x + std::cos(DEG2RAD(it->m_yaw)) * RANGE,
				end.y + std::sin(DEG2RAD(it->m_yaw)) * RANGE,
				end.z };

			// compute the direction.
			Vector dir = end - start;
			float len = dir.Normalize();

			// should never happen.
			if (len <= 0.f)
				continue;

			// step thru the total distance, 4 units per step.
			for (float i{ 0.f }; i < len; i += STEP) {
				// get the current step position.
				Vector point = start + (dir * i);

				// get the contents at this point.
				int contents = Interfaces::m_pEngineTrace->GetPointContents(point, MASK_SHOT_HULL);

				// contains nothing that can stop a bullet.
				if (!(contents & MASK_SHOT_HULL))
					continue;

				float mult = 1.f;

				// over 50% of the total length, prioritize this shit.
				if (i > (len * 0.5f))
					mult = 1.25f;

				// over 90% of the total length, prioritize this shit.
				if (i > (len * 0.75f))
					mult = 1.25f;

				// over 90% of the total length, prioritize this shit.
				if (i > (len * 0.9f))
					mult = 2.f;

				// append 'penetrated distance'.
				it->m_dist += (STEP * mult);

				// mark that we found anything.
				valid = true;
			}
		}

		if (!valid) {
			// set angle to backwards.
			g_ResolverData[player->EntIndex()].m_flBestYaw = angAway.y;
			g_ResolverData[player->EntIndex()].m_flBestDistance = -1.f;
			g_ResolverData[player->EntIndex()].m_bCollectedFreestandData = false;
			return;
		}

		// put the most distance at the front of the container.
		std::sort(angles.begin(), angles.end(),
			[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
				return a.m_dist > b.m_dist;
			});

		// the best angle should be at the front now.
		AdaptiveAngle* best = &angles.front();

		// check if we are not doing a useless change.
		if (best->m_dist != g_ResolverData[player->EntIndex()].m_flBestDistance) {
			// set yaw to the best result.
			g_ResolverData[player->EntIndex()].m_flBestYaw = Math::AngleNormalize(best->m_yaw);
			g_ResolverData[player->EntIndex()].m_flBestDistance = best->m_dist;

			// for later use.
			g_ResolverData[player->EntIndex()].m_bCollectedFreestandData = true;
		}
	}

	void CResolver::MatchShot(C_CSPlayer* data, C_AnimationRecord* record) {
		// do not attempt to do this in nospread mode.
		//if (g_menu.main.config.mode.get() == 1)
		//	return;

		auto anim_data = AnimationSystem::Get()->GetAnimationData(data->m_entIndex);
		if (!anim_data)
			return;

		if (anim_data->m_AnimationRecord.empty())
			return;

		float shoot_time = -1.f;

		auto weapon = (C_WeaponCSBaseGun*)(data->m_hActiveWeapon().Get());

		if (weapon) {
			// with logging this time was always one tick behind.
			// so add one tick to the last shoot time.
			shoot_time = weapon->m_fLastShotTime() + Interfaces::m_pGlobalVars->interval_per_tick;
		}

		// this record has a shot on it.
		if (TIME_TO_TICKS(shoot_time) == TIME_TO_TICKS(record->m_flSimulationTime)) {
			//if (record->m_iChokeTicks <= 2)
			//	record->m_shot = true;

			// more then 1 choke, cant hit pitch, apply prev pitch.
			if (anim_data->m_AnimationRecord.size() >= 2 && !record->m_iChokeTicks <= 2) {
				C_AnimationRecord* previous = &anim_data->m_AnimationRecord[1];

				if (previous && !previous->m_bIsInvalid)
					record->m_angEyeAngles.x = data->m_angEyeAngles().x = previous->m_angEyeAngles.x;
			}
		}
	}

	void CResolver::ResolveAngles(C_CSPlayer* player, C_AnimationRecord* record) {
		FindBestAngle(player);

		// we arrived here we can do the acutal resolve.
		if (record->m_iResolverMode == EResolverModes::RESOLVE_WALK)
			ResolveWalk(player, record);

		else if (record->m_iResolverMode == EResolverModes::RESOLVE_STAND || record->m_iResolverMode == EResolverModes::RESOLVE_PRED)
			ResolveStand(player, record);

		else if (record->m_iResolverMode == EResolverModes::RESOLVE_AIR)
			ResolveAir(player, record);
	}

	void CResolver::ResolveYaw(C_CSPlayer* player, C_AnimationRecord* record) {
		float speed = record->m_vecAnimationVelocity.Length();

		MatchShot(player, record);

		//auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->EntIndex()).Xor();

		//if (!lag_data)
		//	return;

		//std::stringstream msg;


		//printf(std::to_string(lag_data->m_History.size()).c_str());
		//printf("\n");

		if ((record->m_fFlags & FL_ONGROUND) && speed > 0.1f && !(record->m_bFakeWalking /*|| record->m_bUnsafeVelocityTransition*/))
			record->m_iResolverMode = EResolverModes::RESOLVE_WALK;

		if ((record->m_fFlags & FL_ONGROUND) && (speed < 0.1f || record->m_bFakeWalking /*|| record->m_bUnsafeVelocityTransition*/))
			record->m_iResolverMode = EResolverModes::RESOLVE_STAND;

		else if (!(record->m_fFlags & FL_ONGROUND))
			record->m_iResolverMode = EResolverModes::RESOLVE_AIR;

		// attempt to resolve the player.
		ResolveAngles(player, record);

		// write potentially resolved angles.
		record->m_angEyeAngles.y = player->m_angEyeAngles().y = g_ResolverData[player->EntIndex()].m_flFinalResolverYaw;
	}

	void CResolver::ResolveWalk(C_CSPlayer* player, C_AnimationRecord* record) {
		// apply lby to eyeangles.
		g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = record->m_flLowerBodyYawTarget;

		// get lag data.
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->EntIndex());
		if (pLagData.IsValid()) {
			// predict the next time if they stop moving.
			pLagData->m_iMissedShotsLBY = 0;
			pLagData->m_iMissedShots = 0;
		}

		// haven't checked this yet, do it in CResolver::ResolveStand( ... )
		g_ResolverData[player->EntIndex()].m_bCollectedValidMoveData = true;

		// store the data about the moving player, we need to because it contains crucial info
		// that we will have to later on use in our resolver.
		g_ResolverData[player->EntIndex()].m_sMoveData.m_flAnimTime = player->m_flAnimationTime();
		g_ResolverData[player->EntIndex()].m_sMoveData.m_vecOrigin = record->m_vecOrigin;
		g_ResolverData[ player->EntIndex( ) ].m_sMoveData.m_flLowerBodyYawTarget = record->m_flLowerBodyYawTarget;
		g_ResolverData[player->EntIndex()].m_sMoveData.m_flSimulationTime = record->m_flSimulationTime;
	}

	void CResolver::ResolveStand(C_CSPlayer* player, C_AnimationRecord* record) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		// get resolver data.
		auto& data = g_ResolverData[player->EntIndex()];

		// get predicted away angle for the player.
		Vector angAway;
		Math::VectorAngles(local->m_vecOrigin() - record->m_vecOrigin, angAway);

		// get lag data.
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->EntIndex());
		if (!pLagData.IsValid()) {
			return;
		}

		const int nMisses = pLagData->m_iMissedShots;

		// we have a valid moving record.
		static Vector vDormantOrigin;
		if (player->IsDormant()) {
			data.m_bCollectedValidMoveData = false;
			vDormantOrigin = record->m_vecOrigin;
		}
		//else {
		//	Vector delta = vDormantOrigin - record->m_vecOrigin;
		//	if (delta.Length() > 16.f) {
		//		data.m_bCollectedValidMoveData = true;
		//		vDormantOrigin = Vector();
		//	}
		//}

		//if(data.m_bCollectedValidMoveData && IsLastMoveValid(record, g_ResolverData[player->EntIndex()].m_sMoveData.m_flLowerBodyYawTarget) && nMisses < 1)
		//	g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = g_ResolverData[player->EntIndex()].m_sMoveData.m_flLowerBodyYawTarget;

		//// lets fire at freestand angle first.
		//else if( data.m_bCollectedFreestandData )
		//	g_ResolverData[ player->EntIndex( ) ].m_flFinalResolverYaw = g_ResolverData[ player->EntIndex( ) ].m_flBestYaw;

		//// else if we missed 2 shots while in freestand stage.
		//// lets bruteforce.
		//else if( ( data.m_bCollectedFreestandData && nMisses > 2 ) && pLagData->DetectAutoDirerction( pLagData, player ) ) {
		//	g_ResolverData[ player->EntIndex( ) ].m_flFinalResolverYaw = pLagData->m_flDirection;
		//}

		if (g_ResolverData[player->EntIndex()].is_flicking) {
			g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = record->m_flLowerBodyYawTarget;
		}
		else {
			// we have valid move data but we can't freestand them.
			if (data.m_bCollectedValidMoveData && !data.m_bCollectedFreestandData) {
				// https://www.unknowncheats.me/forum/counterstrike-global-offensive/292854-animation-syncing.html
				// we haven't missed a shot and last move is valid.
				if (nMisses < 1 && (IsLastMoveValid(record, g_ResolverData[player->EntIndex()].m_sMoveData.m_flLowerBodyYawTarget) || data.m_bCollectedValidMoveData))
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = g_ResolverData[player->EntIndex()].m_sMoveData.m_flLowerBodyYawTarget;

				// we missed more than 2 shots
				// lets bruteforce since we have no idea where he at.
				else if (nMisses > 0) {
					switch (nMisses % 6) {
					case 1:
						g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = g_ResolverData[player->EntIndex()].m_flBestYaw;
						break;

					case 2:
						g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y + 70.f;
						break;

					case 3:
						g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y - 70.f;
						break;

					case 4:
						g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y + 180.f;
						break;

					case 5:
						g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = record->m_flLowerBodyYawTarget;
						break;

					default:
						break;
					}
				}
			}

			else if (data.m_bCollectedFreestandData && data.m_bCollectedValidMoveData) {
				switch (nMisses % 6) {
				case 0:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = g_ResolverData[player->EntIndex()].m_sMoveData.m_flLowerBodyYawTarget;
					break;
				case 1:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = g_ResolverData[player->EntIndex()].m_flBestYaw;
					break;

				case 2:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y + 70.f;
					break;

				case 3:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y - 70.f;
					break;

				case 4:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y + 180.f;
					break;

				case 5:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = record->m_flLowerBodyYawTarget;
					break;

				default:
					break;
				}
			}


			// if there isn't any valid move data
			// lets bruteforce.
			else if (!data.m_bCollectedFreestandData && !data.m_bCollectedValidMoveData) {
				switch (nMisses % 5) {

				case 0:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y;
					break;

				case 1:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = g_ResolverData[player->EntIndex()].m_flBestYaw;
					break;

				case 2:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y - 70.f;
					break;

				case 3:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y + 70.f;
					break;

				case 4:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = record->m_flLowerBodyYawTarget;
					break;

				default:
					break;
				}
			}

			//shouldn't happen but just in case.
			else {
				switch (nMisses % 5) {
				case 0:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = g_ResolverData[player->EntIndex()].m_flBestYaw;
					break;

				case 1:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y + 70.f;
					break;

				case 2:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y - 70.f;
					break;

				case 3:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = angAway.y + 180.f;
					break;

				case 4:
					g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = record->m_flLowerBodyYawTarget;
					break;

				default:
					break;
				}
			}
		}
	}

	void CResolver::ResolveAir(C_CSPlayer* player, C_AnimationRecord* record) {
		// we have barely any speed. 
		// either we jumped in place or we just left the ground.
		// or someone is trying to fool our resolver.
		if (record->m_vecAnimationVelocity.Length2D() < 60.f) {
			// set this for completion.
			// so the shot parsing wont pick the hits / misses up.
			// and process them wrongly.
			record->m_iResolverMode = EResolverModes::RESOLVE_STAND;

			// invoke our stand resolver.
			ResolveStand(player, record);

			// we are done.
			return;
		}

		// get lag data.
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		// try to predict the direction of the player based on his velocity direction.
		// this should be a rough estimation of where he is looking.
		float velyaw = RAD2DEG(std::atan2(record->m_vecAnimationVelocity.y, record->m_vecAnimationVelocity.x));

		switch (pLagData->m_iMissedShots % 4) {
		case 0:
			g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = velyaw + 180.f;
			break;

		case 1:
			g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = velyaw - 135.f;
			break;

		case 2:
			g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = velyaw + 135.f;
			break;

		case 3:
			g_ResolverData[player->EntIndex()].m_flFinalResolverYaw = velyaw;
			break;
		}
	}

	void CResolver::ResolveManual(C_CSPlayer* player, C_AnimationRecord* record, bool bDisallow) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		static auto bHasTarget = false;
		if (!g_Vars.rage.override_reoslver.enabled) {
			bHasTarget = false;
			return;
		}

		static std::vector<C_CSPlayer*> pTargets;
		static auto bLastChecked = 0.f;

		// check if we have a player?
		if (bLastChecked != Interfaces::m_pGlobalVars->curtime) {
			// update this.
			bLastChecked = Interfaces::m_pGlobalVars->curtime;
			pTargets.clear();

			// get viewangles.
			QAngle m_vecLocalViewAngles;
			Interfaces::m_pEngine->GetViewAngles(m_vecLocalViewAngles);

			// loop through all entitys.
			const auto m_flNeededFOV = 20.f;
			for (int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i) {
				auto entity = reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntity(i));
				if (!entity || entity->IsDead() || !entity->IsTeammate(local))
					continue;

				auto AngleDistance = [&](QAngle& angles, const Vector& start, const Vector& end) -> float {
					auto direction = end - start;
					auto aimAngles = direction.ToEulerAngles();
					auto delta = aimAngles - angles;
					delta.Normalize();

					return sqrtf(delta.x * delta.x + delta.y * delta.y);
				};
				// get distance based FOV.
				float m_flBaseFOV = AngleDistance(m_vecLocalViewAngles, local->GetEyePosition(), entity->GetEyePosition());

				// we have a valid target in our FOV.
				if (m_flBaseFOV < m_flNeededFOV) {
					// push back our target.
					pTargets.push_back(entity);
				}
			}
		}

		// we dont have any targets.
		if (pTargets.empty()) {
			bHasTarget = false;
			return;
		}

		auto bFoundPlayer = false;
		// iterate through our targets.
		for (auto& t : pTargets) {
			if (player == t) {
				bFoundPlayer = true;
				break;
			}
		}

		// we dont have one lets exit.
		if (!bFoundPlayer)
			return;

		// get lag data.
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->EntIndex());
		if (!pLagData.IsValid()) {
			return;
		}

		// get current viewangles.
		QAngle m_vecViewAngles;
		Interfaces::m_pEngine->GetViewAngles(m_vecViewAngles);

		static auto m_flLastDelta = 0.f;
		static auto m_flLastAngle = 0.f;

		const auto angAway = Math::CalcAngle(local->GetEyePosition(), player->GetEyePosition()).y;
		auto m_flDelta = Math::AngleNormalize(m_vecViewAngles.y - angAway);

		if (bHasTarget && fabsf(m_vecViewAngles.y - m_flLastAngle) < 0.1f) {
			m_vecViewAngles.y = m_flLastAngle;
			m_flDelta = m_flLastDelta;
		}

		bHasTarget = true;

		if (g_Vars.rage.override_reoslver.enabled) {
			if (m_flDelta > 1.2f)
				record->m_angEyeAngles.y = angAway + 90.f;
			else if (m_flDelta < -1.2f)
				record->m_angEyeAngles.y = angAway - 90.f;
			else
				record->m_angEyeAngles.y = angAway;
		}

		m_flLastAngle = m_vecViewAngles.y;
		m_flLastDelta = m_flDelta;
	}

	void CResolver::PredictBodyUpdates(C_CSPlayer* player, C_AnimationRecord* record, C_AnimationRecord* prev) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		if (!(local->m_fFlags() & FL_ONGROUND))
			return;

		// nah
		if (local->IsDead()) {
			Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = false;
			g_ResolverData[player->EntIndex()].m_bCollectedValidMoveData = false;
			return;
		}

		// we have no reliable move data, we can't predict
		if (!Engine::g_ResolverData[player->EntIndex()].m_bCollectedValidMoveData) {
			Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = false;
			return;
		}

		//// fake flick shit.
		//if (!record->m_bUnsafeVelocityTransition) {
		//	Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = false;
		//	return;
		//}

		// get lag data about this player
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid()) {
			Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = false;
			return;
		}

		if (pLagData->m_iMissedShotsLBY >= 2) {
			Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = false;
			return;
		}

		// inform esp that we're about to be the prediction process
		Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = true;

		//check if the player is walking
		// no need for the extra fakewalk check since we null velocity when they're fakewalking anyway
		
		if (!player->IsDormant()) {
			// since we null velocity when they fakewalk, no need to check for it.
			if (record->m_vecVelocity.Length() > 0.1f && !record->m_bFakeWalking) {
				Add[player->EntIndex()] = 0.22f;
				NextLBYUpdate[player->EntIndex()] = record->m_anim_time + Add[player->EntIndex()];
				g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = NextLBYUpdate[player->EntIndex()];
			}
			// lby wont update on this tick but after.
			else if (record->m_anim_time >= NextLBYUpdate[player->EntIndex()] && pLagData->m_iMissedShotsLBY < 2)
			{
				record->m_iResolverMode = EResolverModes::RESOLVE_PRED;
				g_ResolverData[player->EntIndex()].is_flicking = true;
				record->m_bResolved = true;
				Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = true;
				Add[player->EntIndex()] = 1.1f;
				NextLBYUpdate[player->EntIndex()] = record->m_anim_time + Add[player->EntIndex()];
				g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = NextLBYUpdate[player->EntIndex()];
			}
			else
				g_ResolverData[player->EntIndex()].is_flicking = false;

			//// LBY updated via PROXY.
			//if (g_ResolverData[player->EntIndex()].m_flLowerBodyYawTarget != g_ResolverData[player->EntIndex()].m_flOldLowerBodyYawTarget) {
			//	is_flicking = true;
			//	record->m_bResolved = true;
			//	Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = true;
			//	Add[player->EntIndex()] = Interfaces::m_pGlobalVars->interval_per_tick + 1.1f;
			//	NextLBYUpdate[player->EntIndex()] = record->m_anim_time + Add[player->EntIndex()];
			//	g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = NextLBYUpdate[player->EntIndex()];
			//}
			//else
			//	is_flicking = false;

			if (record->m_vecVelocity.Length() > 0.1f && !record->m_bFakeWalking) {
				Add[player->EntIndex()] = 0.22f;
				NextLBYUpdate[player->EntIndex()] = record->m_anim_time + Add[player->EntIndex()];
				g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = NextLBYUpdate[player->EntIndex()];

				Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = false;
			}
		}

		 
		//if ( /*record->m_vecAnimationVelocity.Length( ) > 0.1f*/record->m_vecVelocity.Length2D() > 0.1f && !record->m_bFakeWalking) {
		//	// predict the first flick they have to do after they stop moving
		//	Engine::g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = player->m_flAnimationTime() + 0.22f;

		//	// since we are still not in the prediction process, inform the cheat that we arent predicting yet
		//	// this is only really used to determine if we should draw the lby timer bar on esp, no other real purpose
		//	Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = false;
		//}

		//// lby updated on this tick via timing or proxy
		//else if (player->m_flAnimationTime() >= Engine::g_ResolverData[player->EntIndex()].m_flNextBodyUpdate /*|| Engine::g_ResolverData[player->EntIndex()].m_flLowerBodyYawTarget != Engine::g_ResolverData[player->EntIndex()].m_flOldLowerBodyYawTarget*/) {
		//	// inform the cheat of the resolver method
		//	record->m_iResolverMode = EResolverModes::RESOLVE_PRED;

		//	// predict the next body update
		//	Engine::g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = player->m_flAnimationTime() + 1.1f;

		//	// set eyeangles to lby
		//	record->m_angEyeAngles.y = player->m_angEyeAngles().y = Engine::g_ResolverData[player->EntIndex()].m_flLowerBodyYawTarget;

		//	// this is also only really used for esp flag
		//	record->m_bResolved = true;

		//	// we're now in the prediction stage.
		//	Engine::g_ResolverData[player->EntIndex()].m_bPredictingUpdates = true;
		//}
	}
}
