#include "AntiAim.hpp"
#include "../../SDK/CVariables.hpp"
#include "../Miscellaneous/Movement.hpp"
#include "../../source.hpp"
#include "../../Utils/InputSys.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../../SDK/Valve/CBaseHandle.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "LagCompensation.hpp"
#include "Autowall.h"
#include "../Game/SimulationContext.hpp"
#include "../../SDK/displacement.hpp"
#include "../../Renderer/Render.hpp"
#include "TickbaseShift.hpp"
#include "../Visuals/ESP.hpp"
#include <random>
#include "Resolver.hpp"
#include "Ragebot.hpp"

namespace Interfaces
{
	class C_AntiAimbot : public AntiAimbot {
	public:
		void Main(bool* bSendPacket, bool* bFinalPacket, Encrypted_t<CUserCmd> cmd, bool ragebot) override;
		void PrePrediction(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) override;
		void ImposterBreaker(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) override;
	private:
		virtual float GetAntiAimX(Encrypted_t<CVariables::ANTIAIM_STATE> settings);
		virtual float GetAntiAimY(Encrypted_t<CVariables::ANTIAIM_STATE> settings, Encrypted_t<CUserCmd> cmd);

		virtual void Distort(Encrypted_t<CUserCmd> cmd);

		enum class Directions : int {
			YAW_RIGHT = -1,
			YAW_BACK,
			YAW_LEFT,
			YAW_NONE,
		};
		virtual Directions HandleDirection(Encrypted_t<CUserCmd> cmd);

		bool DoEdgeAntiAim(C_CSPlayer* player, QAngle& out);

		void SendFakeFlick();

		bool airstuck();

		void fake_flick(Encrypted_t<CUserCmd> cmd);

		virtual bool IsEnabled(Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings);

		bool m_bNegate = false;
		float m_flLowerBodyUpdateTime = 0.f;
		float m_flLowerBodyUpdateYaw = FLT_MAX;

		float  m_auto;
		float  m_auto_dist;
		float  m_auto_last;
		float  m_view;
		float  m_auto_time;

	};

	//void C_AntiAimbot::SendFakeFlick() {
	//	if (g_TickbaseController.m_didFakeFlick) {
	//		g_TickbaseController.s_nExtraProcessingTicks = 14;
	//		g_TickbaseController.ignoreallcmds = true;
	//		g_TickbaseController.m_didFakeFlick = false;
	//	}
	//}

	void C_AntiAimbot::fake_flick(Encrypted_t<CUserCmd> cmd)
	{
		if (!g_Vars.misc.mind_trick || !g_Vars.misc.mind_trick_bind.enabled || !g_Vars.misc.slow_walk_bind.enabled)
			return;

		auto localPlayer = C_CSPlayer::GetLocalPlayer();

		if (!localPlayer->m_iHealth() > 0)
			return;

		//if (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled) {
		//	if (cmd->sidemove == 0 && cmd->forwardmove == 0 /*&& localPlayer->m_vecVelocity().Length2D() < 10.f*/) {
		//		static bool switcher = false;
		//		cmd->sidemove = switcher ? -9.99f : 9.99f;
		//		switcher = !switcher;
		//	}
		//}

		static bool swap = false;
		swap = !swap;

		auto prevTickCount = cmd->tick_count;

		auto tickAmount = INT_MAX / 8;


		if (g_Vars.globals.updatingPacket && g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.key && g_Vars.misc.slow_walk_bind.enabled) {
			if (/*cmd->sidemove == 0 && cmd->forwardmove == 0 && */localPlayer->m_vecVelocity().Length2D() < 17.f) {
				static bool switcher2 = false;
				cmd->sidemove = switcher2 ? -17.f : 17.f;
				switcher2 = !switcher2;
			}

			static bool switcher = false;
			auto curtime = Interfaces::m_pGlobalVars->curtime + .01f;
			if (Interfaces::m_pGlobalVars->curtime >= curtime) {
				cmd->tick_count = tickAmount;
				auto curtime2 = Interfaces::m_pGlobalVars->curtime + .015f;
				if (Interfaces::m_pGlobalVars->curtime >= curtime2) {
					cmd->tick_count = tickAmount;
					cmd->command_number = INT_MAX;
				}
			}
			cmd->viewangles.y += switcher ? -(g_Vars.misc.mind_trick_factor) : (g_Vars.misc.mind_trick_factor);
			switcher = !switcher;
			//printf("flicking\n");
		}


		//if (g_Vars.globals.updatingPacket) {
		//	cmd->viewangles.y += RandomFloat(135.f, 165.f);
		//	printf("flicking\n");
		//}



		//if (localPlayer->m_vecVelocity().Length2D() < 15.f) {
		//	if (Interfaces::m_pClientState->m_nChokedCommands() == 0) {
		//		if (localPlayer->m_flAnimationTime() < g_Vars.globals.m_flBodyPred) {
		//			if (g_TickbaseController.lastShiftedCmdNr != Interfaces::m_pClientState->m_nLastOutgoingCommand()) {
		//				//static bool switcher2 = false;
		//				//g_cl.m_cmd->m_view_angles.x -= 180.f;
		//				//g_cl.m_cmd->m_view_angles.y += switcher2 ? g_menu.main.antiaim.angleflick.get() : -(g_menu.main.antiaim.angleflick.get());
		//				//switcher2 = !switcher2;
		//				cmd->viewangles.y += 135.f;
		//				//g_cl.ticksToShift = 0;
		//				if (/*g_cl.m_cmd->m_side_move == 0 && g_cl.m_cmd->m_forward_move == 0 && */localPlayer->m_vecVelocity().Length2D() < 11.f) {
		//					static bool switcher = false;
		//					cmd->sidemove = switcher ? -13.37f : 13.37f;
		//					switcher = !switcher;
		//				}
		//				g_TickbaseController.m_didFakeFlick = true;
		//			}
		//		}
		//	}
		//}
	}

	bool C_AntiAimbot::IsEnabled(Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();
		if (!LocalPlayer || LocalPlayer->IsDead())
			return false;

		if (!(g_Vars.antiaim.bomb_activity && g_Vars.globals.BobmActivityIndex == LocalPlayer->EntIndex()) || !g_Vars.antiaim.bomb_activity)
			if ((cmd->buttons & IN_USE) && (!settings->desync_e_hold || LocalPlayer->m_bIsDefusing()))
				return false;

		if (LocalPlayer->m_MoveType() == MOVETYPE_NOCLIP)
			return false;

		static auto g_GameRules = *(uintptr_t**)(Engine::Displacement.Data.m_GameRules);
		if (g_GameRules && *(bool*)(*(uintptr_t*)g_GameRules + 0x20) || (LocalPlayer->m_fFlags() & (1 << 6)))
			return false;

		C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)LocalPlayer->m_hActiveWeapon().Get();

		if (!Weapon)
			return false;

		auto WeaponInfo = Weapon->GetCSWeaponData();
		if (!WeaponInfo.IsValid())
			return false;

		if (WeaponInfo->m_iWeaponType == WEAPONTYPE_GRENADE) {
			if (!Weapon->m_bPinPulled() || (cmd->buttons & (IN_ATTACK | IN_ATTACK2))) {
				float throwTime = Weapon->m_fThrowTime();
				if (throwTime > 0.f)
					return false;
			}
		}
		else {
			if ((WeaponInfo->m_iWeaponType == WEAPONTYPE_KNIFE && cmd->buttons & (IN_ATTACK | IN_ATTACK2)) || cmd->buttons & IN_ATTACK) {
				if (LocalPlayer->CanShoot())
					return false;
			}
		}

		if (LocalPlayer->m_MoveType() == MOVETYPE_LADDER)
			return false;

		return true;
	}

	Encrypted_t<AntiAimbot> AntiAimbot::Get() {
		static C_AntiAimbot instance;
		return &instance;
	}

	std::random_device random;
	std::mt19937 generator(random());
	void C_AntiAimbot::Main(bool* bSendPacket, bool* bFinalPacket, Encrypted_t<CUserCmd> cmd, bool ragebot) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

		if (!LocalPlayer || LocalPlayer->IsDead())
			return;

		auto animState = LocalPlayer->m_PlayerAnimState();
		if (!animState)
			return;

		if (!g_Vars.antiaim.enabled)
			return;

		Encrypted_t<CVariables::ANTIAIM_STATE> settings(&g_Vars.antiaim_stand);

		C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)LocalPlayer->m_hActiveWeapon().Get();

		if (!Weapon)
			return;

		auto WeaponInfo = Weapon->GetCSWeaponData();
		if (!WeaponInfo.IsValid())
			return;

		if (!IsEnabled(cmd, settings))
			return;

		m_auto_time = 4.f;

		if (LocalPlayer->m_MoveType() == MOVETYPE_LADDER) {
			auto eye_pos = LocalPlayer->GetEyePosition();

			CTraceFilterWorldAndPropsOnly filter;
			CGameTrace tr;
			Ray_t ray;
			float angle = 0.0f;
			while (true) {
				float cosa, sina;
				DirectX::XMScalarSinCos(&cosa, &sina, angle);

				Vector pos;
				pos.x = (cosa * 32.0f) + eye_pos.x;
				pos.y = (sina * 32.0f) + eye_pos.y;
				pos.z = eye_pos.z;

				ray.Init(eye_pos, pos,
					Vector(-1.0f, -1.0f, -4.0f),
					Vector(1.0f, 1.0f, 4.0f));
				Interfaces::m_pEngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);
				if (tr.fraction < 1.0f)
					break;

				angle += DirectX::XM_PIDIV2;
				if (angle >= DirectX::XM_2PI) {
					return;
				}
			}

			float v23 = atan2(tr.plane.normal.x, std::fabsf(tr.plane.normal.y));
			float v24 = RAD2DEG(v23) + 90.0f;
			cmd->viewangles.pitch = 89.0f;
			if (v24 <= 180.0f) {
				if (v24 < -180.0f) {
					v24 = v24 + 360.0f;
				}
				cmd->viewangles.yaw = v24;
			}
			else {
				cmd->viewangles.yaw = v24 - 360.0f;
			}

			if (cmd->buttons & IN_BACK) {
				cmd->buttons |= IN_FORWARD;
				cmd->buttons &= ~IN_BACK;
			}
			else  if (cmd->buttons & IN_FORWARD) {
				cmd->buttons |= IN_BACK;
				cmd->buttons &= ~IN_FORWARD;
			}

			return;
		}

		bool move = LocalPlayer->m_vecVelocity().Length2D() > 0.1f && !g_Vars.globals.Fakewalking;

		// save view, depending if locked or not.
		if ((g_Vars.antiaim.freestand_lock && move) || !g_Vars.antiaim.freestand_lock)
			m_view = cmd->viewangles.y;

		cmd->viewangles.x = GetAntiAimX(settings);
		float flYaw = GetAntiAimY(settings, cmd);

		// do not allow 2 consecutive sendpacket true if faking angles.
		if (*bSendPacket && g_Vars.globals.m_bOldPacket)
			*bSendPacket = false;


		static bool swap = false;
		swap = !swap;

		auto prevTickCount = cmd->tick_count;

		auto tickAmount = INT_MAX / 8;

		if (g_Vars.misc.move_exploit && g_Vars.misc.move_exploit_key.enabled) {
			auto prevCommandNumber = cmd->command_number;

			if (swap) {
				g_Vars.fakelag.choke = 14;
			}
			else {
				g_Vars.fakelag.choke = 16;
			}

			if (*bSendPacket == false) {
				cmd->tick_count = tickAmount;
				cmd->command_number = prevCommandNumber;


				//*g_send_packet = true;
			}

			if (*bSendPacket)
			{
				auto curtime = Interfaces::m_pGlobalVars->curtime + .01f;
				if (Interfaces::m_pGlobalVars->curtime >= curtime) {
					cmd->tick_count = tickAmount;
					auto curtime2 = Interfaces::m_pGlobalVars->curtime + .015f;
					if (Interfaces::m_pGlobalVars->curtime >= curtime2) {
						cmd->tick_count = tickAmount;
						cmd->command_number = INT_MAX;
					}
					//g_cmd->command_number = tickAmount;
				}

			}
		}


		// https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/cl_main.cpp#L1877-L1881
		if (!*bSendPacket || !*bFinalPacket) {

			cmd->viewangles.y = flYaw;

			Distort(cmd);

			fake_flick(cmd);
		}
		else {
			//*bSendPacket = true;

			std::uniform_int_distribution random(-90, 90);

			cmd->viewangles.y = Math::AngleNormalize(flYaw + 180 + random(generator));

			//SendFakeFlick();
		}

		static int negative = false;
		auto bSwitch = std::fabs(Interfaces::m_pGlobalVars->curtime - g_Vars.globals.m_flBodyPred) < Interfaces::m_pGlobalVars->interval_per_tick;
		auto bSwap = std::fabs(Interfaces::m_pGlobalVars->curtime - g_Vars.globals.m_flBodyPred) > 1.1 - (Interfaces::m_pGlobalVars->interval_per_tick * 5);
		if (!Interfaces::m_pClientState->m_nChokedCommands()
			&& Interfaces::m_pGlobalVars->curtime >= g_Vars.globals.m_flBodyPred
			&& LocalPlayer->m_fFlags() & FL_ONGROUND && !move) {
			//*bSendPacket = true;
			// fake yaw.
			switch (settings->yaw) {
			case 1: // static
				cmd->viewangles.y += g_Vars.antiaim.break_lby;
				break;
			case 2: // twist
				negative ? cmd->viewangles.y += 110.f : cmd->viewangles.y -= 110.f;
				negative = !negative;
				break;
			default:
				break;
			}

			m_flLowerBodyUpdateYaw = LocalPlayer->m_flLowerBodyYawTarget();
		}

		bool bUsingManualAA = g_Vars.globals.manual_aa != -1;

		if (settings->base_yaw == 2 && g_Vars.globals.m_bGround && !Interfaces::m_pClientState->m_nChokedCommands()) { // jitter
			static auto j = false;

			cmd->viewangles.y += j ? g_Vars.antiaim.Jitter_range : -g_Vars.antiaim.Jitter_range;
			j = !j;

		}

		QAngle ang;

		if (DoEdgeAntiAim(LocalPlayer, ang) && !bUsingManualAA && g_Vars.antiaim.freestand_mode == 1 && g_Vars.antiaim.freestand && g_Vars.globals.m_bGround && !Interfaces::m_pClientState->m_nChokedCommands()) { // run edge aa
			cmd->viewangles.y += Math::AngleNormalize(ang.y);
		}


		/*if ( g_Vars.antiaim.imposta ) {
			Interfaces::AntiAimbot::Get( )->ImposterBreaker( bSendPacket, cmd );
		}*/
	}

	void C_AntiAimbot::PrePrediction(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) {
		if (!g_Vars.antiaim.enabled)
			return;

		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return;

		Encrypted_t<CVariables::ANTIAIM_STATE> settings(&g_Vars.antiaim_stand);

		//g_Vars.globals.m_bInverted = g_Vars.antiaim.desync_flip_bind.enabled;

		if (!IsEnabled(cmd, settings)) {
			g_Vars.globals.RegularAngles = cmd->viewangles;
			return;
		}
	}

	float C_AntiAimbot::GetAntiAimX(Encrypted_t<CVariables::ANTIAIM_STATE> settings) {
		if (!g_Vars.globals.m_rce_forceup) {
			switch (settings->pitch) {
			case 1: // down
				return 89.f;
			case 2: // up 
				return -89.f;
			case 3: // zero
				return 0.f;
			default:
				return FLT_MAX;
				break;
			}
		}
		else if (g_Vars.globals.m_rce_forceup) {
			return -89.f;
		}
	}



	float C_AntiAimbot::GetAntiAimY(Encrypted_t<CVariables::ANTIAIM_STATE> settings, Encrypted_t<CUserCmd> cmd) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return FLT_MAX;

		float flViewAnlge = cmd->viewangles.y;
		float flRetValue = flViewAnlge + 180.f;

		bool bUsingManualAA = g_Vars.globals.manual_aa != -1;

		if (bUsingManualAA) {
			switch (g_Vars.globals.manual_aa) {
			case 0:
				flRetValue = flViewAnlge + 90.f;
				break;
			case 1:
				flRetValue = flViewAnlge + 180.f;
				break;
			case 2:
				flRetValue = flViewAnlge - 90.f;
				break;
			}
		}


		// lets do our real yaw.'
		switch (settings->base_yaw) {
		case 1: { // backwards.
			if (!bUsingManualAA) {
				flRetValue = flViewAnlge + 180.f;
			}
			break;
		}
		case 2: { // jitter

			//static auto last_yaw = 0.f;
			//static auto st = NULL;
			//static auto j = false;


			//flRetValue += j ? g_Vars.antiaim.Jitter_range : -g_Vars.antiaim.Jitter_range;
			//j = !j;
	

			break;
		}
		case 3: { // 180z

			if (!bUsingManualAA) {
				flRetValue = (flViewAnlge - 180.f / 2.f);
				flRetValue += std::fmod(Interfaces::m_pGlobalVars->curtime * (3.5 * 20.f), 180.f);
			}

			break;

		default:
			break;
		}

		}


		if (g_Vars.antiaim.freestand) {


			if (!bUsingManualAA && g_Vars.antiaim.freestand_mode == 0) {
				C_AntiAimbot::Directions Direction = HandleDirection(cmd);
				switch (Direction) {
			case Directions::YAW_BACK:
				// backwards yaw.
				flRetValue = flViewAnlge + 180.f;
				break;
			case Directions::YAW_LEFT:
				// left yaw.
				flRetValue = flViewAnlge + 90.f;
				break;
			case Directions::YAW_RIGHT:
				// right yaw.
				flRetValue = flViewAnlge - 90.f;
				break;
			case Directions::YAW_NONE:
				// 180.
				flRetValue = flViewAnlge + 180.f;
				break;
				}
			}


		}

		//if (!bUsingManualAA && g_Vars.antiaim.preserve) {
		//	if (local->m_vecVelocity().Length2D() > 3.25f && local->m_vecVelocity().Length2D() < 20.f && !g_Vars.globals.Fakewalking) {
		//		flRetValue = flViewAnlge + 180.f;
		//	}
		//}

		static int iUpdates;
		if (iUpdates > pow(10, 10))
			iUpdates = 1;

		if (!g_Vars.globals.m_bGround) {
			flRetValue = flViewAnlge + (iUpdates % 2 ? -155.f : 155.f);
			++iUpdates;
		}

		return flRetValue;
	}

	void C_AntiAimbot::Distort(Encrypted_t<CUserCmd> cmd) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return;

		if (!g_Vars.antiaim.distort)
			return;

		bool bDoDistort = true;
		if (g_Vars.antiaim.distort_disable_fakewalk && g_Vars.globals.Fakewalking)
			bDoDistort = false;

		if (g_Vars.antiaim.distort_disable_air && !g_Vars.globals.m_bGround)
			bDoDistort = false;

		static float flLastMoveTime = FLT_MAX;
		static float flLastMoveYaw = FLT_MAX;
		static bool bGenerate = true;
		static float flGenerated = 0.f;

		if (local->m_PlayerAnimState()->m_velocity > 0.1f && g_Vars.globals.m_bGround && !g_Vars.globals.Fakewalking) {
			flLastMoveTime = Interfaces::m_pGlobalVars->realtime;
			flLastMoveYaw = local->m_flLowerBodyYawTarget();

			if (g_Vars.antiaim.distort_disable_run)
				bDoDistort = false;
		}

		if (g_Vars.globals.manual_aa != -1 && !g_Vars.antiaim.distort_manual_aa)
			bDoDistort = false;

		if (flLastMoveTime == FLT_MAX)
			return;

		if (flLastMoveYaw == FLT_MAX)
			return;

		if (!bDoDistort) {
			bGenerate = true;
		}

		if (bDoDistort) {
			// don't distort for longer than this
			if (fabs(Interfaces::m_pGlobalVars->realtime - flLastMoveTime) > g_Vars.antiaim.distort_max_time && g_Vars.antiaim.distort_max_time > 0.f) {
				return;
			}

			if (g_Vars.antiaim.distort_twist) {
				float flDistortion = std::sin((Interfaces::m_pGlobalVars->realtime * g_Vars.antiaim.distort_speed) * 0.5f + 0.5f);

				cmd->viewangles.y += g_Vars.antiaim.distort_range * flDistortion;
				return;
			}

			if (bGenerate) {
				float flNormalised = std::remainderf(g_Vars.antiaim.distort_range, 360.f);

				flGenerated = RandomFloat(-flNormalised, flNormalised);
				bGenerate = false;
			}

			float flDelta = fabs(flLastMoveYaw - local->m_flLowerBodyYawTarget());
			cmd->viewangles.y += flDelta + flGenerated;
		}
	}

	// CX
	void C_AntiAimbot::ImposterBreaker(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) {
		auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal)
			return;

		bool bCrouching = pLocal->m_PlayerAnimState()->m_fDuckAmount > 0.f;
		float flVelocity = (bCrouching ? 3.25f : 1.01f);
		static int iUpdates = 0;

		if (!(g_Vars.globals.m_pCmd->buttons & IN_FORWARD) && !(g_Vars.globals.m_pCmd->buttons & IN_BACK) && !(g_Vars.globals.m_pCmd->buttons & IN_MOVELEFT) && !(g_Vars.globals.m_pCmd->buttons & IN_MOVERIGHT) && !(g_Vars.globals.m_pCmd->buttons & IN_JUMP))
		{
			if (Interfaces::m_pClientState->m_nChokedCommands() == 2)
			{
				cmd->forwardmove = iUpdates % 2 ? -450 : 450;
				++iUpdates;
			}
		}
	}


	struct AutoTarget_t { float fov; C_CSPlayer* player; };
	C_AntiAimbot::Directions C_AntiAimbot::HandleDirection(Encrypted_t<CUserCmd> cmd) {
		const auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal) return Directions::YAW_NONE;

		// best target.
		AutoTarget_t target{ 180.f + 1.f, nullptr };

		// iterate players, for closest distance.
		for (int i = 1; i <= Interfaces::m_pGlobalVars->maxClients; i++) {
			auto player = C_CSPlayer::GetPlayerByIndex(i);
			if (!player || player->IsDormant() || player == pLocal || player->IsDead() || player->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
			if (!lag_data.IsValid())
				continue;

			auto AngleDistance = [&](QAngle& angles, const Vector& start, const Vector& end) -> float {
				auto direction = end - start;
				auto aimAngles = direction.ToEulerAngles();
				auto delta = aimAngles - angles;
				delta.Normalize();

				return sqrtf(delta.x * delta.x + delta.y * delta.y);
			};

			float fov = AngleDistance(cmd->viewangles, g_Vars.globals.m_vecFixedEyePosition, player->m_vecOrigin());

			if (fov < target.fov) {
				target.fov = fov;
				target.player = player;
			}
		}

		// get best player.
		if (const auto player = target.player)
		{
			Vector& bestOrigin = player->m_vecOrigin();

			// calculate direction from bestOrigin to our origin
			const auto yaw = Math::CalcAngle(bestOrigin, pLocal->m_vecOrigin());

			Vector forward, right, up;
			Math::AngleVectors(yaw, forward, right, up);

			Vector vecStart = pLocal->GetEyePosition();
			Vector vecEnd = vecStart + forward * 100.0f;

			Ray_t rightRay(vecStart + right * 35.0f, vecEnd + right * 35.0f), leftRay(vecStart - right * 35.0f, vecEnd - right * 35.0f);

			// setup trace filter
			CTraceFilter filter{ };
			filter.pSkip = pLocal;

			CGameTrace tr{ };

			m_pEngineTrace->TraceRay(rightRay, MASK_SOLID, &filter, &tr);
			float rightLength = (tr.endpos - tr.startpos).Length();

			m_pEngineTrace->TraceRay(leftRay, MASK_SOLID, &filter, &tr);
			float leftLength = (tr.endpos - tr.startpos).Length();

			static auto leftTicks = 0;
			static auto rightTicks = 0;
			static auto backTicks = 0;

			if (rightLength - leftLength > 20.0f)
				leftTicks++;
			else
				leftTicks = 0;

			if (leftLength - rightLength > 20.0f)
				rightTicks++;
			else
				rightTicks = 0;

			if (fabs(rightLength - leftLength) <= 20.0f)
				backTicks++;
			else
				backTicks = 0;

			Directions direction = Directions::YAW_NONE;

			if (rightTicks > 10) {
				direction = Directions::YAW_RIGHT;
			}
			else {
				if (leftTicks > 10) {
					direction = Directions::YAW_LEFT;
				}
				else {
					if (backTicks > 10)
						direction = Directions::YAW_BACK;
				}
			}

			return direction;
		}

		return Directions::YAW_NONE;
	}

	bool C_AntiAimbot::DoEdgeAntiAim(C_CSPlayer* player, QAngle& out) {
		CGameTrace trace;
		static CTraceFilter filter{ };
		Ray_t ray;

		if (player->m_MoveType() == MOVETYPE_LADDER)
			return false;

		// skip this player in our traces.
		filter.pSkip = player;

		// get player bounds.
		Vector mins = player->OBBMins();
		Vector maxs = player->OBBMaxs();

		// make player bounds bigger.
		mins.x -= 20.f;
		mins.y -= 20.f;
		maxs.x += 20.f;
		maxs.y += 20.f;

		// get player origin.
		Vector start = player->GetAbsOrigin();

		// offset the view.
		start.z += 56.f;

		ray.Init(start, start, mins, maxs);

		Interfaces::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);


		if (!trace.startsolid)
			return false;

		float  smallest = 1.f;
		Vector plane;

		// trace around us in a circle, in 20 steps (anti-degree conversion).
		// find the closest object.
		for (float step{ }; step <= Math::pi_2; step += (Math::pi / 10.f)) {
			// extend endpoint x units.
			Vector end = start;

			// set end point based on range and step.
			end.x += std::cos(step) * 32.f;
			end.y += std::sin(step) * 32.f;

			ray.Init(start, end, { -1.f, -1.f, -8.f }, { 1.f, 1.f, 8.f });

			Interfaces::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);

			// we found an object closer, then the previouly found object.
			if (trace.fraction < smallest) {
				// save the normal of the object.
				plane = trace.plane.normal;
				smallest = trace.fraction;
			}
		}

		// no valid object was found.
		if (smallest == 1.f || plane.z >= 0.1f)
			return false;

		// invert the normal of this object
		// this will give us the direction/angle to this object.
		Vector inv = Vector(-plane.x, -plane.y, -plane.z);
		Vector dir = inv;
		dir.Normalize();

		// extend point into object by 24 units.
		Vector point = start;
		point.x += (dir.x * 24.f);
		point.y += (dir.y * 24.f);

		// check if we can stick our head into the wall.
		if (Interfaces::m_pEngineTrace->GetPointContents(point, CONTENTS_SOLID) & CONTENTS_SOLID) {
			// trace from 72 units till 56 units to see if we are standing behind something.

			ray.Init(point + Vector{ 0.f, 0.f, 16.f }, point);

			Interfaces::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);

			// we didnt start in a solid, so we started in air.
			// and we are not in the ground.
			if (trace.fraction < 1.f && !trace.startsolid && trace.plane.normal.z > 0.7f) {
				// mean we are standing behind a solid object.
				// set our angle to the inversed normal of this object.
				out.y = Math::rad_to_deg(std::atan2(inv.y, inv.x));
				return true;
			}
		}

		// if we arrived here that mean we could not stick our head into the wall.
		// we can still see if we can stick our head behind/asides the wall.

		// adjust bounds for traces.
		mins = { (dir.x * -3.f) - 1.f, (dir.y * -3.f) - 1.f, -1.f };
		maxs = { (dir.x * 3.f) + 1.f, (dir.y * 3.f) + 1.f, 1.f };

		// move this point 48 units to the left 
		// relative to our wall/base point.
		Vector left = start;
		left.x = point.x - (inv.y * 48.f);
		left.y = point.y - (inv.x * -48.f);

		ray.Init(left, point, mins, maxs);

		Interfaces::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
		float l = trace.startsolid ? 0.f : trace.fraction;

		// move this point 48 units to the right 
		// relative to our wall/base point.
		Vector right = start;
		right.x = point.x + (inv.y * 48.f);
		right.y = point.y + (inv.x * -48.f);

		ray.Init(right, point, mins, maxs);

		Interfaces::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
		float r = trace.startsolid ? 0.f : trace.fraction;

		// both are solid, no edge.
		if (l == 0.f && r == 0.f)
			return false;

		// set out to inversed normal.
		out.y = Math::rad_to_deg(std::atan2(inv.y, inv.x));

		// left started solid.
		// set angle to the left.
		if (l == 0.f) {
			out.y += 90.f;
			return true;
		}

		// right started solid.
		// set angle to the right.
		if (r == 0.f) {
			out.y -= 90.f;
			return true;
		}

		return false;
	}

}