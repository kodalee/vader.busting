#include "Resolver.hpp"
#include "../../SDK/CVariables.hpp"
#include "../Visuals/CChams.hpp"
#include "../Rage/AntiAim.hpp"
#include "../Rage/Ragebot.hpp"
#include "../Rage/Autowall.h"
#include "../Visuals/EventLogger.hpp"
#include "Ragebot.hpp"
#include "TickbaseShift.hpp"

static float NextLBYUpdate[65];
static float Add[65];
bool is_flicking;

C_CSPlayer* get_entity(const int index) { return reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntity(index)); }

namespace Engine {
	CResolver g_Resolver;
	CResolverData g_ResolverData[65];

	void CResolver::collect_wall_detect(const ClientFrameStage_t stage)
	{
		if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			return;

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local || !local->IsAlive())
			return;

		last_eye_positions.insert(last_eye_positions.begin(), local->m_vecOrigin() + local->m_vecViewOffset());
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
			auto player = get_entity(i);

			if (!player || player == local)
			{
				continue;
			}

			if (player->IsTeammate(local))
			{
				continue;
			}

			if (!player->IsAlive())
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

			auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
			if (!anim_data)
				return;

			if (anim_data->m_AnimationRecord.empty())
				return;

			if (!anim_data->m_AnimationRecord.empty() && player->m_flSimulationTime() - anim_data->m_AnimationRecord[0].m_flSimulationTime == 0)
				continue;

			//if (anim_data->m_AnimationRecord[0].m_vecLastNonDormantOrig != player->m_vecOrigin() && local->IsAlive())
			//{
			//	g_ResolverData[player->EntIndex()].m_iMode = 1;
			//}
			//else {
			//	g_ResolverData[player->EntIndex()].m_iMode = 0;
			//}

			//if (player->m_flSimulationTime() - anim_data->m_flLastLowerBodyYawTargetUpdateTime > 1.35f && log.m_vecLastNonDormantOrig == player->get_origin() && log.m_iMode == RMODE_MOVING)
			//{
			//	if (player->get_simtime() - log.m_flLastLowerBodyYawTargetUpdateTime > 1.65f)
			//	{
			//		log.m_iMode = RMODE_WALL;
			//	}
			//}

			//if (g_ResolverData[player->EntIndex()].m_iMode == 1)
			//{
			const auto at_target_angle = Math::CalcAngle(player->m_vecOrigin(), last_eye);

			//Vector left_dir, right_dir, back_dir;
			//Math::angle_vectors(Vector(0.f, at_target_angle.y - 90.f, 0.f), &left_dir);
			//Math::angle_vectors(Vector(0.f, at_target_angle.y + 90.f, 0.f), &right_dir);
			//Math::angle_vectors(Vector(0.f, at_target_angle.y + 180.f, 0.f), &back_dir);


			Vector direction_1, direction_2, direction_3;
			Math::AngleVectors(QAngle(0.f, Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y - 90.f, 0.f), &direction_1);
			Math::AngleVectors(QAngle(0.f, Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y + 90.f, 0.f), &direction_2);
			Math::AngleVectors(QAngle(0.f, Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y + 180.f, 0.f), &direction_3);

			const float height = 64;

			const auto left_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_1 * 16.f);
			const auto right_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_2 * 16.f);
			const auto back_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_3 * 16.f);

			//log.anti_freestanding_record.left_damage = penetration::get().get_damage(latency_based_eye_pos,
			anti_freestanding_record.left_damage = Autowall::ScaleDamage(player, left_damage[i], 1.f, Hitgroup_Head);

			//anti_freestanding_record.left_damage = penetration::scale(player, &left_damage[i],
			//	HITGROUP_CHEST);
			//data->anti_freestanding_record.right_damage = FEATURES::RAGEBOT::autowall.CalculateDamage(latency_based_eye_pos,
			//	right_eye_pos, local_player, entity, 1).damage;
			anti_freestanding_record.right_damage = Autowall::ScaleDamage(player, right_damage[i], 1.f, Hitgroup_Head);
			//penetration::get().get_damage(g_cl.m_local, player, right_eye_pos, &right_damage[i],
			//get_big_fucking_gun(), &latency_based_eye_pos);
		//BACKWARDS
			anti_freestanding_record.back_damage = Autowall::ScaleDamage(player, back_damage[i], 1.f, Hitgroup_Head);

			Ray_t ray;
			CGameTrace trace;
			CTraceFilterWorldOnly filter;

			Ray_t first_ray(left_eye_pos, latency_based_eye_pos);
			Interfaces::m_pEngineTrace->TraceRay(first_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.left_fraction = trace.fraction;

			Ray_t second_ray(right_eye_pos, latency_based_eye_pos);
			Interfaces::m_pEngineTrace->TraceRay(second_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.right_fraction = trace.fraction;

			Ray_t third_ray(back_eye_pos, latency_based_eye_pos);
			Interfaces::m_pEngineTrace->TraceRay(third_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.back_fraction = trace.fraction;

			//const auto eye_pos = player->get_eye_pos();
			//auto left_eye_pos = eye_pos + (left_dir * 16.f);
			//auto right_eye_pos = eye_pos + (right_dir * 16.f);
			//auto back_eye_pos = eye_pos + (back_dir * 16.f);

			//static CCSWeaponData big_fucking_gun{};
			//auto get_big_fucking_gun = [&]() -> CCSWeaponData*
			//{
			//	big_fucking_gun.iDamage = 200;
			//	big_fucking_gun.flRangeModifier = 1.0f;
			//	big_fucking_gun.flPenetration = 6.0f;
			//	big_fucking_gun.flArmorRatio = 2.0f;
			//	big_fucking_gun.flRange = 8192.f;
			//	return &big_fucking_gun;
			//};

			//penetration::get().get_damage(g_pLocalPlayer, player, left_eye_pos, &left_damage[i], get_big_fucking_gun(), &last_eye);
			//penetration::get().get_damage(g_pLocalPlayer, player, right_eye_pos, &right_damage[i], get_big_fucking_gun(), &last_eye);
			//penetration::get().get_damage(g_pLocalPlayer, player, back_eye_pos, &back_damage[i], get_big_fucking_gun(), &last_eye);
		//}
		}
	}

	bool CResolver::wall_detect(C_CSPlayer* player, C_AnimationRecord* record, float& angle) const
	{

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local->IsAlive())
			return false;

		//if (player == local)
		//	return false;

		//printf("BALLS\n");

		const auto at_target_angle = Math::CalcAngle(record->m_vecOrigin, last_eye);

		auto set = false;

		const auto left = left_damage[player->EntIndex()];
		const auto right = right_damage[player->EntIndex()];
		const auto back = back_damage[player->EntIndex()];

		auto max_dmg = std::max(left, std::max(right, back)) - 1.f;
		if (left < max_dmg)
		{
			max_dmg = left;
			angle = Math::normalize_float(at_target_angle.y + 90.f);
			set = true;
		}
		if (right < max_dmg)
		{
			max_dmg = right;
			angle = Math::normalize_float(at_target_angle.y - 90.f);
			set = true;
		}
		if (back < max_dmg || !set)
		{
			max_dmg = back;
			angle = Math::normalize_float(at_target_angle.y + 180.f);
		}

		return true;
	}

	void CResolver::AntiFreestand(C_AnimationRecord* record, C_CSPlayer* entity) {

		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(entity->m_entIndex);
		if (!pLagData.IsValid())
			return;

		if (entity->EntIndex() == local->EntIndex())
			return;

		C_AnimationRecord* move = &pLagData->m_walk_record;

		// constants
		constexpr float STEP{ 4.f };
		constexpr float RANGE{ 32.f };

		// best target.
		Vector enemypos = entity->GetShootPosition();
		float away = GetAwayAngle(record);

		// construct vector of angles to test.
		std::vector< AdaptiveAngle > angles{ };
		angles.emplace_back(away - 180.f);
		angles.emplace_back(away + 90.f);
		angles.emplace_back(away - 90.f);

		// start the trace at the your shoot pos.
		Vector start = local->GetShootPosition();

		// see if we got any valid result.
		// if this is false the path was not obstructed with anything.
		bool valid{ false };

		// iterate vector of angles.
		for (auto it = angles.begin(); it != angles.end(); ++it) {

			// compute the 'rough' estimation of where our head will be.
			Vector end{ enemypos.x + std::cos(Math::deg_to_rad(it->m_yaw)) * RANGE,
				enemypos.y + std::sin(Math::deg_to_rad(it->m_yaw)) * RANGE,
				enemypos.z };

			// draw a line for debugging purposes.
			// g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

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

		if (!valid || !wall_detect(entity, record, record->m_angEyeAngles.y)) {
			if (record->m_moved && pLagData->m_iMissedShots < 2) {
				record->m_angEyeAngles.y = move->m_body;
				record->m_iResolverText = XorStr("LASTMOVE");
			}
			else {
				record->m_angEyeAngles.y = away + 180.f;
				record->m_iResolverText = XorStr("BACKWARDS");

			}
			return;
		}

		// put the most distance at the front of the container.
		std::sort(angles.begin(), angles.end(),
			[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
				return a.m_dist > b.m_dist;
			});

		// the best angle should be at the front now.
		AdaptiveAngle* best = &angles.front();

		record->m_angEyeAngles.y = best->m_yaw;
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

	static float NextLBYUpdate[65];
	static float Add[65];

	void CResolver::SetMode(C_AnimationRecord* record) {
		// the resolver has 3 modes to chose from.
		// these modes will vary more under the hood depending on what data we have about the player
		// and what kind of hack vs. hack we are playing (mm/nospread).

		float speed = record->m_vecVelocity.Length2D();

		// if on ground, moving, and not fakewalking.
		if ((record->m_fFlags & FL_ONGROUND) && speed > 0.1f && !record->m_bFakeWalking)
			record->m_iResolverMode = RESOLVE_WALK;

		//if (g_Vars.rage.override_reoslver.enabled && record->m_fFlags & FL_ONGROUND && (speed <= 25.f || record->m_bFakeWalking))
		//	record->m_iResolverMode = RESOLVE_OVERRIDE;

		// if on ground, not moving or fakewalking.
		else if ((record->m_fFlags & FL_ONGROUND) && (speed <= 0.1f || record->m_bFakeWalking))
			record->m_iResolverMode = RESOLVE_LASTMOVE;

		// if not on ground.
		else if (!(record->m_fFlags & FL_ONGROUND))
			record->m_iResolverMode = RESOLVE_AIR;
	}

	void CResolver::ResolveAngles(C_CSPlayer* player, C_AnimationRecord* record) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		if (pLagData->m_bRoundStart) {
			record->m_moved = false;
			printf("move reset\n");
		}

		C_AnimationRecord* move = &pLagData->m_walk_record;

		// mark this record if it contains a shot.
		MatchShot(player, record);

		// next up mark this record with a resolver mode that will be used.
		SetMode(record);

		// if we are in nospread mode, force all players pitches to down.
		// TODO; we should check thei actual pitch and up too, since those are the other 2 possible angles.
		// this should be somehow combined into some iteration that matches with the air angle iteration.

		//static float test = 0.f;

		//if (player->m_vecVelocity().Length2D() < 0.1f)
			//test = player->m_AnimOverlay()[3].m_flCycle;

		//printf(std::to_string(test).c_str());

		// we arrived here we can do the acutal resolve.
		if (record->m_iResolverMode == RESOLVE_WALK)
			ResolveWalk(player, record);

		//else if (record->m_iResolverMode == RESOLVE_OVERRIDE || g_Vars.rage.override_reoslver.enabled)
		//	ResolveOverride(player, record);

		else if (record->m_iResolverMode == RESOLVE_LASTMOVE || record->m_iResolverMode == RESOLVE_UNKNOWM)
			LastMoveLby(record, player);

		else if (record->m_iResolverMode == RESOLVE_AIR)
			ResolveAir(player, record);

		if (!local->IsAlive() || !player->IsAlive() || player->IsDormant())
			record->m_moved = false;

		// normalize the eye angles, doesn't really matter but its clean.
		Math::NormalizeAngle(record->m_angEyeAngles.y);

		player->m_angEyeAngles() = record->m_angEyeAngles;
	}

	void CResolver::OnBodyUpdate(C_CSPlayer* player, float value) {
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		// set data.
		pLagData->m_old_body = pLagData->m_body;
		pLagData->m_body = value;
	}

	void CResolver::ResolveWalk(C_CSPlayer* data, C_AnimationRecord* record) {
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(data->m_entIndex);
		if (!pLagData.IsValid())
			return;

		record->m_iResolverText = XorStr("MOVING");
		// apply lby to eyeangles.
		record->m_angEyeAngles.y = record->m_body;

		// delay body update.
		//data->m_flLowerBodyYawTarget_update = record->m_anim_time + 0.22f;

		float speed = record->m_vecVelocity.Length2D();

		// reset stand and body index.
		if (speed > 20.f && !record->m_bFakeWalking)
			pLagData->m_iMissedShotsLBY = 0;


		if (record->m_fFlags & FL_ONGROUND && speed > 1.f && data->m_fFlags() & FL_ONGROUND && !record->m_bFakeWalking) {
			//printf("move true 1\n");
			record->m_moved = true;
		}

		pLagData->m_iMissedShots = 0;
		pLagData->m_stand_index2 = 0;
		pLagData->m_iMissedShotsLBY = 0;
		pLagData->m_last_move = 0;
		pLagData->m_unknown_move = 0;
		pLagData->m_delta_index = 0;

		// copy the last record that this player was walking
		// we need it later on because it gives us crucial data.
		std::memcpy(&pLagData->m_walk_record, record, sizeof(C_AnimationRecord));
	}

	float CResolver::GetLBYRotatedYaw(float lby, float yaw)
	{
		float delta = Math::NormalizedAngle(yaw - lby);
		if (fabs(delta) < 25.f)
			return lby;

		if (delta > 0.f)
			return yaw + 25.f;

		return yaw;
	}

	float CResolver::GetAwayAngle(C_AnimationRecord* record) {
		float  delta{ std::numeric_limits< float >::max() };
		Vector pos;
		QAngle  away;

		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return 0.f;

		// other cheats predict you by their own latency.
		// they do this because, then they can put their away angle to exactly
		// where you are on the server at that moment in time.

		// the idea is that you would need to know where they 'saw' you when they created their user-command.
		// lets say you move on your client right now, this would take half of our latency to arrive at the server.
		// the delay between the server and the target client is compensated by themselves already, that is fortunate for us.

		// we have no historical origins.
		// no choice but to use the most recent one.
		//if( g_cl.m_net_pos.empty( ) ) {
		Math::VectorAngles(local->m_vecOrigin() - record->m_vecOrigin, away);
		return away.y;
		//}

		// half of our rtt.
		// also known as the one-way delay.
		//float owd = ( g_cl.m_latency / 2.f );

		// since our origins are computed here on the client
		// we have to compensate for the delay between our client and the server
		// therefore the OWD should be subtracted from the target time.
		//float target = record->m_pred_time; //- owd;

		// iterate all.
		//for( const auto &net : g_cl.m_net_pos ) {
			// get the delta between this records time context
			// and the target time.
		//	float dt = std::abs( target - net.m_time );

			// the best origin.
		//	if( dt < delta ) {
		//		delta = dt;
		//		pos   = net.m_pos;
		//	}
		//}

		//math::VectorAngles( pos - record->m_pred_origin, away );
		//return away.y;
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
			yaw = at_target_yaw;

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
				yaw = at_target_yaw;
			}
		}
		else
		{
			if (freestanding_record.left_damage > freestanding_record.right_damage) {
				yaw = at_target_yaw + 130.f;
				set = true;
			}
			else
				yaw = at_target_yaw;
		}

		return true;
	}

	void CResolver::LastMoveLby(C_AnimationRecord* record, C_CSPlayer* player)
	{
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
		if (!anim_data)
			return;

		// pointer for easy access.
		C_AnimationRecord* move = &pLagData->m_walk_record;

		// get predicted away angle for the player.
		float away = GetAwayAngle(record);

		C_AnimationLayer* curr = &record->m_serverAnimOverlays[3];
		int act = player->GetSequenceActivity(curr->m_nSequence);

		float diff = Math::NormalizedAngle(record->m_body - move->m_body);
		float delta = record->m_anim_time - move->m_anim_time;

		//if (diff < -35.f || diff > 35.f)
		//	printf("Not breaking\n");

		QAngle vAngle = QAngle(0, 0, 0);
		Math::CalcAngle3(player->m_vecOrigin(), local->m_vecOrigin(), vAngle);

		float flToMe = vAngle.y;

		auto index = player->EntIndex();

		const float at_target_yaw = Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

		const auto freestanding_record = player_resolve_records[player->EntIndex()].m_sAntiEdge;

		if (move->m_flSimulationTime > 0.f) {
			if (!record->m_moved) {
				Vector delta = move->m_vecOrigin - record->m_vecOrigin;
				if (delta.Length() <= 128.f && record->m_fFlags & FL_ONGROUND) {
					record->m_moved = true;
					//printf("move true 2\n");
				}
			}
		}

		if (player->IsDormant()) {
			is_flicking = false;
		}

		// predict LBY flicks.
		if (!player->IsDormant() /*&& !record->dormant()*/) {
			// since we null velocity when they fakewalk, no need to check for it.
			if (record->m_vecAnimationVelocity.Length() > 0.1f) {
				Add[player->EntIndex()] = 0.22f;
				NextLBYUpdate[player->EntIndex()] = record->m_anim_time + Add[player->EntIndex()];
				record->m_body_update = NextLBYUpdate[player->EntIndex()];
			}
			// lby wont update on this tick but after.
			if (record->m_anim_time >= NextLBYUpdate[player->EntIndex()] && record->m_moved/*&& !player->IsDormant()*//* && !record->dormant()*/)
			{
				is_flicking = true;
				Add[player->EntIndex()] = 1.1f;
				NextLBYUpdate[player->EntIndex()] = record->m_anim_time + Add[player->EntIndex()];
				record->m_body_update = NextLBYUpdate[player->EntIndex()];
			}
			else
				is_flicking = false;

			if (pLagData->m_body != pLagData->m_old_body && record->m_moved/*&& !record->dormant()*/) {
				is_flicking = true;
				Add[player->EntIndex()] = Interfaces::m_pGlobalVars->interval_per_tick + 1.1f;
				NextLBYUpdate[player->EntIndex()] = Interfaces::m_pGlobalVars->interval_per_tick + Add[player->EntIndex()];
				record->m_body_update = NextLBYUpdate[player->EntIndex()];
			}

			if (record->m_vecVelocity.Length() > 0.1f && !record->m_bFakeWalking) {
				Add[player->EntIndex()] = 0.22f;
				NextLBYUpdate[player->EntIndex()] = record->m_anim_time + Add[player->EntIndex()];
				record->m_body_update = NextLBYUpdate[player->EntIndex()];
			}
		}
		else {
			is_flicking = false;
		}
		//if (hitPlayer[index] && (player->m_vecVelocity().length_2d() < 1.f || player->m_vecVelocity().length_2d() > 1.f && record->m_bFakeWalking)) {
		//	static bool repeat[64];
		//	if (!repeat[index]) {
		//		data->storedLbyDelta[index] = math::normalize_float(record->m_angEyeAngles.y - data->m_flLowerBodyYawTarget);
		//		data->hasStoredLby[index] = true;
		//		repeat[index] = true;

		//	}
		//	if (repeat[index]) {
		//		data->hasStoredLby[index] = true;
		//	}
		//}
		//else {
		//	data->hasStoredLby[index] = false;
		//}

		//if (data->hasStoredLby[index] && (player->m_vecVelocity().length_2d() < 1.f || player->m_vecVelocity().length_2d() > 1.f && record->m_bFakeWalking) && data->m_delta_index == 0 /*&& !CResolver::get().update_lby_timer(pEntity)*/) {
		//	record->m_iResolverMode = RESOLVE_DELTA;
		//	record->resolver_text = "DELTA";
		//	record->m_angEyeAngles.y = math::normalize_float(data->m_flLowerBodyYawTarget + data->storedLbyDelta[index]);
		//}



		//if (diff > 35.f || diff < -35.f && pLagData->m_lby_index < 1)
		//{
		//	record->m_angEyeAngles.y = record->m_flLowerBodyYawTarget;
		//	record->m_iResolverText = "LBY";
		//}
		//else {

		//C_AnimationRecord* previous = &anim_data->m_AnimationRecord[1];
		//if (previous) {
		//	if (record->m_flLowerBodyYawTarget == previous->m_flLowerBodyYawTarget)
		//		printf("Not breaking\n");
		//}



			if (!record->m_moved) {

				record->m_iResolverMode = RESOLVE_UNKNOWM;
				record->m_iResolverText = XorStr("STAND");


				const float at_target_yaw = Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

				if (is_flicking && pLagData->m_iMissedShotsLBY < 3/* && !record->m_bFakeWalking*/)
				{
					//m_iMode = 0;
					record->m_angEyeAngles.y = record->m_body;

					//data->m_flLowerBodyYawTarget_update = record->m_anim_time + 1.1f;

					record->m_iResolverMode = RESOLVE_BODY;
					record->m_iResolverText = XorStr("UPDATE");
				}
				else {
					switch (pLagData->m_unknown_move % 4) {
					case 0:
						//AntiFreestand(record, player);
						//m_iMode = 1;
						//record->m_iResolverText = XorStr("FREESTAND");
						//if (g_ResolverData->hitPlayer[index] && (player->m_vecVelocity().Length2D() < 0.1f || player->m_vecVelocity().Length2D() > 0.1f && record->m_bFakeWalking)) {
						//	static bool repeat[64];
						//	if (!repeat[index]) {
						//		g_ResolverData->storedLbyDelta[index] = Math::normalize_float(record->m_angEyeAngles.y - record->m_body);
						//		g_ResolverData->hasStoredLby[index] = true;
						//		repeat[index] = true;
						//	}
						//	if (repeat[index]) {
						//		g_ResolverData->hasStoredLby[index] = true;
						//	}
						//}
						//else {
						//	g_ResolverData->hasStoredLby[index] = false;
						//}

						//if (g_ResolverData->hasStoredLby[index] && (player->m_vecVelocity().Length2D() < 0.1f || player->m_vecVelocity().Length2D() > 0.1f && record->m_bFakeWalking) /*&& !resolver::get().update_lby_timer(pEntity)*/) {
						//	record->m_angEyeAngles.y = Math::normalize_float(record->m_body + g_ResolverData->storedLbyDelta[index]);
						//	record->m_iResolverText = XorStr("LBY LOGGED");
						//	g_ResolverData->m_iMode = 32;
						//}
						//else {
							AntiFreestand(record, player);
							g_ResolverData->m_iMode = 1;
							record->m_iResolverText = XorStr("FREESTAND");
						//}
						break;
					case 1:
						record->m_angEyeAngles.y = at_target_yaw + 180.f;
						m_iMode = 0;
						break;
					case 2:
						record->m_angEyeAngles.y = (at_target_yaw + 180.f) + 70.f;
						m_iMode = 0;
						break;
					case 3:
						record->m_angEyeAngles.y = (at_target_yaw + 180.f) - 70.f;
						m_iMode = 0;
						break;
					}
				}

				//if (AntiFreestanding(player, data, record->m_angEyeAngles.y)) {
				//	m_iMode = 1;
				//	//g_notify.add(XOR("ANTIFREESTANDING\n"));
				//}
				//else
				//	m_iMode = 0;
				////SupremAntiFreestanding(record);

			}
			else if (record->m_moved) {
				float diff = Math::NormalizedAngle(record->m_body - move->m_body);
				float delta = record->m_anim_time - move->m_anim_time;
				C_AnimationLayer* curr = &record->m_serverAnimOverlays[3];
				const int activity = player->GetSequenceActivity(curr->m_nSequence);


				record->m_iResolverMode = RESOLVE_LASTMOVE;
				record->m_iResolverText = XorStr("LASTMOVE");

				const float at_target_yaw = Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

				if (is_flicking && pLagData->m_iMissedShotsLBY < 3/* && !record->m_bFakeWalking*/)
				{
					//printf("break detection\n");
					//printf("1\n");
					record->m_angEyeAngles.y = record->m_body;

					//data->m_flLowerBodyYawTarget_update = record->m_anim_time + 1.1f;
					record->m_iResolverMode = RESOLVE_BODY;
					record->m_iResolverText = XorStr("UPDATE");
				}
				else {
					switch (pLagData->m_last_move % 5) {
					case 0:
						//if (g_ResolverData->hitPlayer[index] && (player->m_vecVelocity().Length2D() < 0.1f || player->m_vecVelocity().Length2D() > 0.1f && record->m_bFakeWalking)) {
						//	static bool repeat[64];
						//	if (!repeat[index]) {
						//		g_ResolverData->storedLbyDelta[index] = Math::normalize_float(record->m_angEyeAngles.y - record->m_body);
						//		g_ResolverData->hasStoredLby[index] = true;
						//		repeat[index] = true;
						//	}
						//	if (repeat[index]) {
						//		g_ResolverData->hasStoredLby[index] = true;
						//	}
						//}
						//else {
						//	g_ResolverData->hasStoredLby[index] = false;
						//}

						//if (g_ResolverData->hasStoredLby[index] && (player->m_vecVelocity().Length2D() < 0.1f || player->m_vecVelocity().Length2D() > 5.f && record->m_bFakeWalking) /*&& !resolver::get().update_lby_timer(pEntity)*/) {
						//	record->m_angEyeAngles.y = Math::normalize_float(record->m_body + g_ResolverData->storedLbyDelta[index]);
						//	record->m_iResolverText = XorStr("LBY LOGGED");
						//	g_ResolverData->m_iMode = 32;
						//}
						//else {
							AntiFreestand(record, player);
							g_ResolverData->m_iMode = 1;
							record->m_iResolverText = XorStr("LAST FREESTAND");
						//}
						break;
					case 1:
						AntiFreestand(record, player);
						m_iMode = 1;
						record->m_iResolverText = XorStr("FREESTAND");
						break;
					case 2:
						record->m_angEyeAngles.y = at_target_yaw + 180.f;
						break;
					case 3:
						record->m_angEyeAngles.y = (at_target_yaw + 180.f) + 70.f;
						m_iMode = 0;
						break;
					case 4:
						record->m_angEyeAngles.y = (at_target_yaw + 180.f) - 70.f;
						m_iMode = 0;
						break;
					}
				}
			}
		//}
	}

	//void CResolver::ResolvePoses(C_CSPlayer* player, C_AnimationRecord* record) {
	//	// only do this bs when in air.
	//	if (record->m_iResolverMode == RESOLVE_AIR) {
	//		// ang = pose min + pose val x ( pose range )

	//		// lean_yaw
	//		player->m_flPoseParameter()[2] = RandomInt(0, 4) * 0.25f;

	//		// body_yaw
	//		player->m_flPoseParameter()[11] = RandomInt(1, 3) * 0.25f;
	//	}
	//}

	void CResolver::ResolveAir(C_CSPlayer* player, C_AnimationRecord* record) {
		// get lag data.
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		// we have barely any speed. 
		// either we jumped in place or we just left the ground.
		// or someone is trying to fool our resolver.
		if (record->m_vecAnimationVelocity.Length2D() < 60.f) {
			// set this for completion.
			// so the shot parsing wont pick the hits / misses up.
			// and process them wrongly.
			record->m_iResolverMode = EResolverModes::RESOLVE_STAND;

			// invoke our stand resolver.
			LastMoveLby(record, player);

			// we are done.
			return;
		}

		record->m_iResolverText = XorStr("AIR");

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

	//void CResolver::ResolveOverride(C_CSPlayer* player, C_AnimationRecord* record) {

	//	Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
	//	if (!pLagData.IsValid())
	//		return;

	//	auto local = C_CSPlayer::GetLocalPlayer();
	//	if (!local)
	//		return;

	//	auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
	//	if (!anim_data)
	//		return;

	//	// get predicted away angle for the player.
	//	float away = GetAwayAngle(record);

	//	// pointer for easy access.
	//	C_AnimationRecord* move = &pLagData->m_walk_record;

	//	C_AnimationLayer* curr = &record->m_serverAnimOverlays[3];
	//	int act = player->GetSequenceActivity(curr->m_nSequence);

	//	if (g_Vars.rage.override_reoslver.enabled) {
	//		QAngle viewangles;
	//		Interfaces::m_pEngine->GetViewAngles(viewangles);

	//		//auto yaw = math::clamp (g_cl.m_local->GetAbsOrigin(), Player->origin()).y;
	//		const float at_target_yaw = Math::VecCalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

	//		if (fabs(Math::NormalizedAngle(viewangles.y - at_target_yaw)) > 30.f)
	//			return LastMoveLby(record, player);

	//		record->m_angEyeAngles.y = (Math::NormalizedAngle(viewangles.y - at_target_yaw) > 0) ? at_target_yaw + 90.f : at_target_yaw - 90.f;

	//		//return UTILS::GetLBYRotatedYaw(entity->m_flLowerBodyYawTarget(), (math::NormalizedAngle(viewangles.y - at_target_yaw) > 0) ? at_target_yaw + 90.f : at_target_yaw - 90.f);

	//		record->m_iResolverMode = RESOLVE_OVERRIDE;
	//	}
	//}
}
