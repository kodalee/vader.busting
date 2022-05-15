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
#include "FakeLag.hpp"

#define min2(a, b) (((a) < (b)) ? (a) : (b))

namespace Interfaces
{
	class C_AntiAimbot : public AntiAimbot {
	public:
		void fake_duck(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) override;
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

		void AutoDirection(Encrypted_t<CUserCmd> cmd);

		void do_at_target(Encrypted_t<CUserCmd> cmd);

		void FakeFlick(Encrypted_t<CUserCmd> cmd, bool* bSendPacket);

		bool airstuck();

		//void fake_flick(Encrypted_t<CUserCmd> cmd);

		void lby_prediction(Encrypted_t<CUserCmd> cmd, bool* bSendPacket);

		bool lby_update(Encrypted_t<CUserCmd> cmd, bool* bSendPacket);

		bool do_lby(Encrypted_t<CUserCmd> cmd, bool* bSendPacket);

		virtual bool IsEnabled(Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings);

		bool m_bNegate = false;
		float m_flLowerBodyUpdateTime = 0.f;
		float m_flLowerBodyUpdateYaw = FLT_MAX;

		bool   EdgeFlick = false;
		float  m_auto;
		float  m_auto_dist;
		float  m_auto_last;
		float  m_view;
		float  m_auto_time;

		bool broke_lby = false;
		float next_lby_update = 0.f;
		bool update_lby = false;
		float initial_lby = 0.f;
		float target_lby = 0.f;
		bool firstupdate = true;
		bool secondupdate = true;
	};

	static float next_lby_update_time = 0;

	void C_AntiAimbot::lby_prediction(Encrypted_t<CUserCmd> cmd, bool* bSendPacket)
	{
		auto localPlayer = C_CSPlayer::GetLocalPlayer();

		if (!localPlayer)
			return;


		const auto animstate = localPlayer->m_PlayerAnimState();
		if (!animstate)
			return;

		static int negative = false;

		if (Interfaces::m_pClientState->m_nChokedCommands() || (g_Vars.misc.mind_trick_bind.enabled && g_Vars.misc.mind_trick))
			return;

		if (animstate->m_velocity > 0.1f)
		{
			g_Vars.globals.m_flBodyPred =+ 0.22f;
			firstupdate = true;
		}
		else if (Interfaces::m_pGlobalVars->curtime > g_Vars.globals.m_flBodyPred)
		{
			update_lby = true;
		}

		const auto get_add_by_choke = [&]() -> float
		{
			static auto max = 137.f;
			static auto min = 100.f;

			auto mult = 1.f / 0.2f * TICKS_TO_TIME(Interfaces::m_pClientState->m_nChokedCommands());

			return 100.f + (max - min) * mult;
		};

		Encrypted_t<CVariables::ANTIAIM_STATE> settings(&g_Vars.antiaim_stand);

		if (firstupdate && animstate->m_velocity <= 0.1f || firstupdate && animstate->m_vecVelocity.Length() <= 0.1f && settings->yaw == 1)
		{
			initial_lby = cmd->viewangles.y + g_Vars.antiaim.break_lby_first;

			if (g_Vars.globals.Fakewalking) {
				*bSendPacket = true;
			}
			secondupdate = true;
			firstupdate = false;
		}

		if (!firstupdate && Interfaces::m_pGlobalVars->curtime + TICKS_TO_TIME(Interfaces::m_pClientState->m_nChokedCommands() + 1) > g_Vars.globals.m_flBodyPred
			&& fabsf(Math::normalize_float(cmd->viewangles.y - initial_lby)) < get_add_by_choke() && settings->yaw == 1)
		{
			//cmd->viewangles.y = initial_lby + get_add_by_choke();

			cmd->viewangles.y = initial_lby + get_add_by_choke();

			if (g_Vars.globals.Fakewalking) {
				*bSendPacket = true;
			}

		}

	}


	bool C_AntiAimbot::lby_update(Encrypted_t<CUserCmd> cmd, bool* bSendPacket)
	{
		auto localPlayer = C_CSPlayer::GetLocalPlayer();

		if (!localPlayer)
			return false;

		if (Interfaces::m_pClientState->m_nChokedCommands() || !(localPlayer->m_fFlags() & FL_ONGROUND) || (g_Vars.misc.mind_trick_bind.enabled && g_Vars.misc.mind_trick))
			return false;

		Encrypted_t<CVariables::ANTIAIM_STATE> settings(&g_Vars.antiaim_stand);

		const auto updated = update_lby;

		if (update_lby && settings->yaw == 1)
		{

			auto angles = cmd->viewangles.y;
			if (g_Vars.globals.Fakewalking) {
				*bSendPacket = true;
			}
			target_lby = initial_lby;
			cmd->viewangles.y = initial_lby;
			cmd->viewangles.Clamp();
			update_lby = false;

			if (secondupdate)
			{
				initial_lby = angles + g_Vars.antiaim.break_lby;

				secondupdate = false;
			}
		}

		return updated;
	}

	bool C_AntiAimbot::do_lby(Encrypted_t<CUserCmd> cmd, bool* bSendPacket)
	{
		lby_prediction(cmd, bSendPacket);

		return lby_update(cmd, bSendPacket);
	}

	void C_AntiAimbot::FakeFlick(Encrypted_t<CUserCmd> cmd, bool* bSendPacket) {
		if (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled) {
			auto localPlayer = C_CSPlayer::GetLocalPlayer();

			if (!localPlayer || !localPlayer->IsAlive())
				return;

			if (localPlayer->m_vecVelocity().Length2D() < 15.f) {


				static bool FlickCheck = false;
				static bool MicroMoveSide = false;
				float flViewAnlge = cmd->viewangles.y;
				static bool switcher = false;
				//cmd->viewangles.y = flViewAnlge + (EdgeFlick ? 0 : g_Vars.misc.mind_trick_invert.enabled ? -90 : 90);
				*bSendPacket = !(cmd->tick_count % 2 == 0);
				if (cmd->tick_count % 2 == 0) {
					if (FlickCheck) {
						cmd->viewangles.y += 120;
						FlickCheck = false;
						if (cmd->sidemove == 0) {
							MicroMoveSide = !MicroMoveSide;
							cmd->sidemove = MicroMoveSide ? 11 : -11;
						}
						return;
					}
					if (cmd->tick_count % 18 == 0) {
						FlickCheck = true;
						if (cmd->sidemove == 0) {
							MicroMoveSide = !MicroMoveSide;
							cmd->sidemove = MicroMoveSide ? 11 : -11;
						}
						cmd->viewangles.y -= 115;
						return;
					}
				}
				else if (cmd->sidemove == 0) {
					MicroMoveSide = !MicroMoveSide; // cant remember why i put this here???? dementia machport moment
					cmd->sidemove = MicroMoveSide ? 1.1 : -1.1;
				}
			}
		}
	}


	//void C_AntiAimbot::fake_flick(Encrypted_t<CUserCmd> cmd)
	//{
	//	static bool balls = false;
	//	static bool balls2 = false;

	//	static auto curtime = Interfaces::m_pGlobalVars->curtime + TICKS_TO_TIME(1);

	//	if (!balls) {
	//		curtime = Interfaces::m_pGlobalVars->curtime + TICKS_TO_TIME(1);
	//		balls = true;
	//	}

	//	if (g_Vars.misc.mind_trick_bind.enabled) {
	//		if (!balls2) {
	//			balls = false;
	//			balls2 = true;
	//		}
	//	}
	//	else {
	//		balls2 = false;
	//	}

	//	if (g_Vars.misc.mind_trick_bind.enabled && g_Vars.misc.mind_trick) {

	//		auto localPlayer = C_CSPlayer::GetLocalPlayer();

	//		if (localPlayer->IsDead())
	//			return;


	//		if (localPlayer->m_vecVelocity().Length2D() < 15.f) {
	//			if (Interfaces::m_pClientState->m_nChokedCommands() == 0) {
	//				if (g_Vars.globals.m_flAnimTime < g_Vars.globals.m_flBodyPred) {
	//					if (Interfaces::m_pGlobalVars->curtime >= curtime) {
	//						g_Vars.globals.shift_amount = 16;
	//						//g_TickbaseController.m_didFakeFlick = true;
	//						static bool switcher = false;
	//						cmd->viewangles.y += g_Vars.misc.mind_trick_factor;
	//						switcher = !switcher;
	//						g_Vars.globals.shift_amount = 0;
	//						//g_TickbaseController.m_didFakeFlick = false;
	//						curtime = Interfaces::m_pGlobalVars->curtime + TICKS_TO_TIME(1);
	//						//printf("flicking\n");
	//					}
	//					if (localPlayer->m_vecVelocity().Length2D() < 11.f) {
	//						static bool switcher = false;
	//						cmd->sidemove = switcher ? -13.37f : 13.37f;
	//						switcher = !switcher;
	//					}
	//					//g_TickbaseController.m_didFakeFlick = true;

	//					//if (g_TickbaseController.m_didFakeFlick) {
	//					//	g_Vars.globals.shift_amount = 0;
	//					//	g_TickbaseController.m_didFakeFlick = false;
	//					//}
	//				}
	//			}
	//		}
	//	}
	//}

	//	////if (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled) {
	//	////	if (cmd->sidemove == 0 && cmd->forwardmove == 0 /*&& localPlayer->m_vecVelocity().Length2D() < 10.f*/) {
	//	////		static bool switcher = false;
	//	////		cmd->sidemove = switcher ? -9.99f : 9.99f;
	//	////		switcher = !switcher;
	//	////	}
	//	////}

	//	//static bool swap = false;
	//	//swap = !swap;

	//	//auto prevTickCount = cmd->tick_count;

	//	//auto tickAmount = INT_MAX / 8;


	//	//if (g_Vars.globals.updatingPacket && g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.key && g_Vars.misc.slow_walk_bind.enabled) {
	//	//	if (/*cmd->sidemove == 0 && cmd->forwardmove == 0 && */localPlayer->m_vecVelocity().Length2D() < 17.f) {
	//	//		static bool switcher2 = false;
	//	//		cmd->sidemove = switcher2 ? -17.f : 17.f;
	//	//		switcher2 = !switcher2;
	//	//	}

	//	//	static bool switcher = false;
	//	//	auto curtime = Interfaces::m_pGlobalVars->curtime + .01f;
	//	//	if (Interfaces::m_pGlobalVars->curtime >= curtime) {
	//	//		cmd->tick_count = tickAmount;
	//	//		auto curtime2 = Interfaces::m_pGlobalVars->curtime + .015f;
	//	//		if (Interfaces::m_pGlobalVars->curtime >= curtime2) {
	//	//			cmd->tick_count = tickAmount;
	//	//			cmd->command_number = INT_MAX;
	//	//		}
	//	//	}
	//	//	cmd->viewangles.y += switcher ? -(g_Vars.misc.mind_trick_factor) : (g_Vars.misc.mind_trick_factor);
	//	//	switcher = !switcher;
	//	//	//printf("flicking\n");
	//	//}


	//	////if (g_Vars.globals.updatingPacket) {
	//	////	cmd->viewangles.y += RandomFloat(135.f, 165.f);
	//	////	printf("flicking\n");
	//	////}



	//	////if (localPlayer->m_vecVelocity().Length2D() < 15.f) {
	//	////	if (Interfaces::m_pClientState->m_nChokedCommands() == 0) {
	//	////		if (localPlayer->m_flAnimationTime() < g_Vars.globals.m_flBodyPred) {
	//	////			if (g_TickbaseController.lastShiftedCmdNr != Interfaces::m_pClientState->m_nLastOutgoingCommand()) {
	//	////				//static bool switcher2 = false;
	//	////				//g_cl.m_cmd->m_view_angles.x -= 180.f;
	//	////				//g_cl.m_cmd->m_view_angles.y += switcher2 ? g_menu.main.antiaim.angleflick.get() : -(g_menu.main.antiaim.angleflick.get());
	//	////				//switcher2 = !switcher2;
	//	////				cmd->viewangles.y += 135.f;
	//	////				//g_cl.ticksToShift = 0;
	//	////				if (/*g_cl.m_cmd->m_side_move == 0 && g_cl.m_cmd->m_forward_move == 0 && */localPlayer->m_vecVelocity().Length2D() < 11.f) {
	//	////					static bool switcher = false;
	//	////					cmd->sidemove = switcher ? -13.37f : 13.37f;
	//	////					switcher = !switcher;
	//	////				}
	//	////				g_TickbaseController.m_didFakeFlick = true;
	//	////			}
	//	////		}
	//	////	}
	//	////}
	//}

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

	void C_AntiAimbot::fake_duck(bool* bSendPacket, Encrypted_t<CUserCmd> cmd)
	{
		int fakelag_limit = 16;
		int choked_goal = fakelag_limit / 2;
		bool should_crouch = Interfaces::m_pClientState->m_nChokedCommands() >= choked_goal;

		if (should_crouch)
			cmd->buttons |= IN_DUCK;
		else
			cmd->buttons &= ~IN_DUCK;

		*bSendPacket = Interfaces::m_pClientState->m_nChokedCommands() >= fakelag_limit;
	}

	void C_AntiAimbot::Main(bool* bSendPacket, bool* bFinalPacket, Encrypted_t<CUserCmd> cmd, bool ragebot) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

		if (!LocalPlayer || LocalPlayer->IsDead())
			return;

		auto animState = LocalPlayer->m_PlayerAnimState();
		if (!animState)
			return;

		if (!g_Vars.antiaim.enabled)
			return;

		if (g_Vars.misc.balls)
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

		m_auto_time = g_Vars.antiaim.timeout_time;

		static auto fakelagTrack = (int)g_Vars.fakelag.choke;

		static bool restoreFakelag = false;

        if (g_Vars.rage.dt_exploits && g_Vars.rage.key_dt.enabled) {

			//if (g_Vars.misc.mind_trick_test && g_Vars.misc.mind_trick_bind.enabled) {
			//	g_Vars.globals.shift_amount = Interfaces::m_pGlobalVars->tickcount % 16 ? 16 : 0;
			//}

			/*else*/ if (g_Vars.rage.exploit_lag) {
				g_Vars.globals.shift_amount = Interfaces::m_pGlobalVars->tickcount % 16 > 0 ? 16 : 0;
			}

			if (g_Vars.rage.exploit_lagcomp) {

				auto iChoked = Interfaces::m_pClientState->m_nChokedCommands();
				auto Speed2D = LocalPlayer->m_vecVelocity().Length2D();
				int shift_time = 0;
				bool bOnGround = LocalPlayer->m_fFlags() & FL_ONGROUND;


				if (Speed2D > 72)
					shift_time = bOnGround ? 1 : 5;

				if (++shift_time > 16)
					shift_time = 0;

				if (shift_time > iChoked)
					std::clamp(shift_time, 1, 4);

				if (shift_time > 0)
				{
					*bSendPacket = true;
					g_Vars.globals.shift_amount = shift_time > 0 ? 16 : 0;
				}

				//*bSendPacket = false;

				++shift_time = 14;
			}
		}

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

		auto prevTickCount = cmd->tick_count;

		auto tickAmount = INT_MAX / g_Vars.misc.move_exploit_intensity;

		if (g_Vars.misc.move_exploit && g_Vars.misc.move_exploit_key.enabled) {
			auto prevCommandNumber = cmd->command_number;

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

			//if (!g_Vars.misc.mind_trick_bind.enabled) {
				Distort(cmd);
			//}

			FakeFlick(cmd, bSendPacket);
		
			//fake_flick(cmd);

		}
		else {
			//*bSendPacket = true;

			std::uniform_int_distribution random(-110, 110);

			cmd->viewangles.y = Math::AngleNormalize(flYaw + 180 + random(generator));

			//SendFakeFlick();
		}

		// one tick before the update.
		//if (!Interfaces::m_pClientState->m_nChokedCommands() && LocalPlayer->m_fFlags() & FL_ONGROUND && !move && Interfaces::m_pGlobalVars->curtime >= (g_Vars.globals.m_flBodyPred - g_Vars.globals.m_flAnimFrame) && Interfaces::m_pGlobalVars->curtime < g_Vars.globals.m_flBodyPred) {
		//	// z mode.
		//	if (settings->yaw == 3)
		//		cmd->viewangles.y -= 90.f;
		//}

		if ((g_Vars.antiaim.anti_lastmove && !g_Vars.globals.WasShootingInPeek && LocalPlayer->m_fFlags() & FL_ONGROUND && !(cmd->buttons & IN_JUMP) && LocalPlayer->m_vecVelocity().Length() >= 1.2f)) {
			if ((!(cmd->buttons & IN_JUMP) && cmd->forwardmove == cmd->sidemove && cmd->sidemove == 0.0f)) {
				g_Vars.globals.need_break_lastmove = true;
			}
		}

		if (!Interfaces::m_pClientState->m_nChokedCommands() && g_Vars.antiaim.anti_lastmove && g_Vars.globals.need_break_lastmove && !g_Vars.globals.Fakewalking) { // not perfect and does not work alot of times but when it works its good.
			//printf("fuck you kids\n");
			cmd->forwardmove = 7.f;
			*bSendPacket = true;
			cmd->viewangles.y += g_Vars.antiaim.break_lby_first;
			g_Vars.globals.need_break_lastmove = false;
		}

		static int negative = false;
		static bool breaker = false;
		auto bSwitch = std::fabs(Interfaces::m_pGlobalVars->curtime - g_Vars.globals.m_flBodyPred) < Interfaces::m_pGlobalVars->interval_per_tick;
		auto bSwap = std::fabs(Interfaces::m_pGlobalVars->curtime - g_Vars.globals.m_flBodyPred) > 1.1 - (Interfaces::m_pGlobalVars->interval_per_tick * 5);
		//if (!Interfaces::m_pClientState->m_nChokedCommands()
		//	&& Interfaces::m_pGlobalVars->curtime >= g_Vars.globals.m_flBodyPred
		//	&& LocalPlayer->m_fFlags() & FL_ONGROUND && !move) {
		//	if (g_Vars.globals.Fakewalking) {
		//		*bSendPacket = true;
		//	}
		//	// fake yaw.
		//	switch (settings->yaw) {
		//	case 1: // static
		//		/*g_Vars.misc.mind_trick_bind.enabled ? cmd->viewangles.y += g_Vars.misc.mind_trick_lby :*/ cmd->viewangles.y += g_Vars.antiaim.break_lby;
		//		//if (!breaker) {
		//		if(g_Vars.antiaim.flickup)
		//			cmd->viewangles.x -= g_Vars.antiaim.break_lby;
		//		//	breaker = true;
		//		//}
		//		//else if (breaker)
		//		//	breaker = false;
		//		break;
		//	case 2: // twist
		//		negative ? cmd->viewangles.y += 110.f : cmd->viewangles.y -= 110.f;
		//		negative = !negative;
		//		break;
		//	case 3: // z
		//		cmd->viewangles.y += 90.f;
		//		break;
		//	default:
		//		break;
		//	}

		//	m_flLowerBodyUpdateYaw = LocalPlayer->m_flLowerBodyYawTarget();
		//}

		do_lby(cmd, bSendPacket);

		bool bUsingManualAA = g_Vars.globals.manual_aa != -1 && g_Vars.antiaim.manual;

		if (settings->base_yaw == 3 && g_Vars.globals.m_bGround && !Interfaces::m_pClientState->m_nChokedCommands()) { // jitter
			static auto j = false;

			cmd->viewangles.y += j ? g_Vars.antiaim.Jitter_range : -g_Vars.antiaim.Jitter_range;
			j = !j;

		}

		if (settings->base_yaw == 2 && g_Vars.globals.m_bGround && !Interfaces::m_pClientState->m_nChokedCommands()) { // rotate
			cmd->viewangles.y += std::fmod(Interfaces::m_pGlobalVars->curtime * (g_Vars.antiaim.rot_speed * 20.f), g_Vars.antiaim.rot_range);
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

		auto GetTargetYaw = [local, flViewAnlge]() -> float
		{
			float_t flBestDistance = FLT_MAX;

			C_CSPlayer* pFinalPlayer = nullptr;
			for (int32_t i = 1; i < 65; i++)
			{
				C_CSPlayer* pPlayer = C_CSPlayer::GetPlayerByIndex(i);
				if (!pPlayer || !pPlayer->IsPlayer() || pPlayer->IsDead() || pPlayer->m_iTeamNum() == local->m_iTeamNum() || pPlayer->IsDormant())
					continue;

				if (pPlayer->m_fFlags() & FL_FROZEN)
					continue;

				float_t flDistanceToPlayer = local->m_vecOrigin().Distance(pPlayer->m_vecOrigin());
				if (flDistanceToPlayer > flBestDistance)
					continue;

				if (flDistanceToPlayer > 1250.0f)
					continue;

				flBestDistance = flDistanceToPlayer;
				pFinalPlayer = pPlayer;
			}

			if (!pFinalPlayer)
				return flViewAnlge;

			return Math::CalcAngle(local->GetAbsOrigin() + local->m_vecViewOffset(), pFinalPlayer->GetAbsOrigin()).yaw + 180.0f;
		};

		float flRetValue = (g_Vars.antiaim.at_targets ? GetTargetYaw() : flViewAnlge) + 180.f;

		bool bUsingManualAA = g_Vars.globals.manual_aa != -1 && g_Vars.antiaim.manual;

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
		case 4: { // 180z

			if (!bUsingManualAA) {
				flRetValue = (flViewAnlge - 180.f / 2.f);
				flRetValue += std::fmod(Interfaces::m_pGlobalVars->curtime * (3.5 * 20.f), 180.f);
			}

			break;

		default:
			break;
		}

		}

		//if (!bUsingManualAA && g_Vars.antiaim.at_targets) {
		//	do_at_target(cmd);
		//}

		float freestandingReturnYaw = std::numeric_limits< float >::max();

		if (g_Vars.antiaim.freestand) {
			bool DoFreestanding = true;
			QAngle ang;

			if (g_Vars.antiaim.freestand_disable_fakewalk && g_Vars.globals.Fakewalking)
				DoFreestanding = false;

			if (g_Vars.antiaim.freestand_disable_air && !g_Vars.globals.m_bGround)
				DoFreestanding = false;

			if (local->m_PlayerAnimState()->m_velocity > 0.1f && g_Vars.globals.m_bGround && !g_Vars.globals.Fakewalking) {
				if (g_Vars.antiaim.freestand_disable_run)
					DoFreestanding = false;
			}

			if (DoFreestanding && !bUsingManualAA && g_Vars.antiaim.freestand_mode == 0) {
				AutoDirection(cmd);
				flRetValue = m_auto + g_Vars.antiaim.add_yaw;
			}
			else if (DoFreestanding && DoEdgeAntiAim(local, ang) && !bUsingManualAA && g_Vars.antiaim.freestand_mode == 1) { // run edge aa
				flRetValue = Math::AngleNormalize(ang.y);
			}


			//if (!bUsingManualAA && g_Vars.antiaim.freestand_mode == 0) {
			//	C_AntiAimbot::Directions Direction = HandleDirection(cmd);
			//	switch (Direction) {
			//case Directions::YAW_BACK:
			//	// backwards yaw.
			//	flRetValue = flViewAnlge + 180.f;
			//	break;
			//case Directions::YAW_LEFT:
			//	// left yaw.
			//	flRetValue = flViewAnlge + 90.f;
			//	break;
			//case Directions::YAW_RIGHT:
			//	// right yaw.
			//	flRetValue = flViewAnlge - 90.f;
			//	break;
			//case Directions::YAW_NONE:
			//	// 180.
			//	flRetValue = flViewAnlge + 180.f;
			//	break;
			//	}
			//}
		}

		//if (!bUsingManualAA && g_Vars.antiaim.preserve) {
		//	if (local->m_vecVelocity().Length2D() > 3.25f && local->m_vecVelocity().Length2D() < 20.f && !g_Vars.globals.Fakewalking) {
		//		flRetValue = flViewAnlge + 180.f;
		//	}
		//}

		//static int iUpdates;
		//if (iUpdates > pow(10, 10))
		//	iUpdates = 1;

		if (!g_Vars.globals.m_bGround && g_Vars.antiaim.backwards_in_air) {
			flRetValue = flViewAnlge + 180.f;
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

			float flDistortion = std::sin((Interfaces::m_pGlobalVars->realtime * g_Vars.antiaim.distort_speed) * 0.5f + 0.5f);
			float flDelta = fabs(flLastMoveYaw - local->m_flLowerBodyYawTarget());
			cmd->viewangles.y += flDelta + flGenerated * flDistortion;
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

		if (player->m_MoveType() == MOVETYPE_LADDER)
			return false;

		// skip this player in our traces.
		static CTraceFilterSkipEntity filter(player);

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

		Interfaces::m_pEngineTrace->TraceRay(Ray_t(start, start, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
		if (!trace.startsolid)
			return false;

		float  smallest = 1.f;
		Vector plane;

		// trace around us in a circle, in 20 steps (anti-degree conversion).
		// find the closest object.
		for (float step{ }; step <= (M_PI * 2.f); step += (M_PI / 10.f)) {
			// extend endpoint x units.
			Vector end = start;

			// set end point based on range and step.
			end.x += std::cos(step) * 32.f;
			end.y += std::sin(step) * 32.f;

			Interfaces::m_pEngineTrace->TraceRay(Ray_t(start, end, { -1.f, -1.f, -8.f }, { 1.f, 1.f, 8.f }), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);

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
		Vector inv = -plane;
		Vector dir = inv;
		dir.Normalize();

		// extend point into object by 24 units.
		Vector point = start;
		point.x += (dir.x * 24.f);
		point.y += (dir.y * 24.f);

		// check if we can stick our head into the wall.
		if (Interfaces::m_pEngineTrace->GetPointContents(point, CONTENTS_SOLID) & CONTENTS_SOLID) {
			// trace from 72 units till 56 units to see if we are standing behind something.
			Interfaces::m_pEngineTrace->TraceRay(Ray_t(point + Vector{ 0.f, 0.f, 16.f }, point), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);

			// we didnt start in a solid, so we started in air.
			// and we are not in the ground.
			if (trace.fraction < 1.f && !trace.startsolid && trace.plane.normal.z > 0.7f) {
				// mean we are standing behind a solid object.
				// set our angle to the inversed normal of this object.
				out.y = RAD2DEG(std::atan2(inv.y, inv.x));
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

		Interfaces::m_pEngineTrace->TraceRay(Ray_t(left, point, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
		float l = trace.startsolid ? 0.f : trace.fraction;

		// move this point 48 units to the right 
		// relative to our wall/base point.
		Vector right = start;
		right.x = point.x + (inv.y * 48.f);
		right.y = point.y + (inv.x * -48.f);

		Interfaces::m_pEngineTrace->TraceRay(Ray_t(right, point, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
		float r = trace.startsolid ? 0.f : trace.fraction;

		// both are solid, no edge.
		if (l == 0.f && r == 0.f)
			return false;

		// set out to inversed normal.
		out.y = RAD2DEG(std::atan2(inv.y, inv.x));

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

	void C_AntiAimbot::AutoDirection(Encrypted_t<CUserCmd> cmd) {
		const auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal) return;

		// constants.
		constexpr float STEP{ 4.f };
		constexpr float RANGE{ 32.f };

		// best target.
		struct AutoTarget_t { float fov; C_CSPlayer* player; };
		AutoTarget_t target{ 180.f + 1.f, nullptr };

		// iterate players.
		for (int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i) {
			auto player = C_CSPlayer::GetPlayerByIndex(i);

			//skip invalid or unwanted
			if (!player || player->IsDormant() || player == pLocal || player->IsDead() || player->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			// validate player.
			auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
			if (!lag_data.IsValid())
				continue;

			// get best target based on fov.
			float fov = Math::GetFov(cmd->viewangles, pLocal->GetShootPosition(), player->WorldSpaceCenter());

			if (fov < target.fov) {
				target.fov = fov;
				target.player = player;
			}
		}

		if (!target.player) {
			// we have a timeout.
			if (m_auto_last > 0.f && m_auto_time > 0.f && Interfaces::m_pGlobalVars->curtime < (m_auto_last + m_auto_time))
				return;

			// set angle to backwards.
			m_auto = Math::NormalizedAngle(m_view - 180.f);
			m_auto_dist = -1.f;
			return;
		}

		/*
		* data struct
		* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
		*/

		// construct vector of angles to test.
		std::vector< AdaptiveAngle > angles{ };
		angles.emplace_back(m_view - 180.f);
		angles.emplace_back(m_view + 90.f);
		angles.emplace_back(m_view - 90.f);

		// start the trace at the enemy shoot pos.
		Vector start = target.player->GetShootPosition();

		// see if we got any valid result.
		// if this is false the path was not obstructed with anything.
		bool valid{ false };

		// iterate vector of angles.
		for (auto it = angles.begin(); it != angles.end(); ++it) {

			// compute the 'rough' estimation of where our head will be.
			Vector end{ pLocal->GetShootPosition().x + std::cos(Math::deg_to_rad(it->m_yaw)) * RANGE,
				pLocal->GetShootPosition().y + std::sin(Math::deg_to_rad(it->m_yaw)) * RANGE,
				pLocal->GetShootPosition().z };

			// draw a line for debugging purposes.
			//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

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

				// over 75% of the total length, prioritize this shit.
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
			m_auto = Math::NormalizedAngle(m_view - 180.f);
			m_auto_dist = -1.f;
			EdgeFlick = false;
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
		if (best->m_dist != m_auto_dist) {
			// set yaw to the best result.
			m_auto = Math::NormalizedAngle(best->m_yaw);
			m_auto_dist = best->m_dist;
			m_auto_last = Interfaces::m_pGlobalVars->curtime;
			EdgeFlick = true;
		}
	}

	void C_AntiAimbot::do_at_target(Encrypted_t<CUserCmd> cmd)
	{
		const auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal) return;

		C_CSPlayer* target = nullptr;
		QAngle target_angle;

		QAngle original_viewangles;
		Interfaces::m_pEngine->GetViewAngles(original_viewangles);

		auto lowest_fov = 90.f;
		for (auto i = 1; i < Interfaces::m_pGlobalVars->maxClients; i++)
		{
			auto player = C_CSPlayer::GetPlayerByIndex(i);

			//skip invalid or unwanted
			if (!player || player->IsDormant() || player == pLocal || player->IsDead() || player->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			if (player->IsDormant() && (player->m_flSimulationTime() > Interfaces::m_pGlobalVars->curtime || player->m_flSimulationTime() + 5.f < Interfaces::m_pGlobalVars->curtime))
				continue;

			auto enemy_pos = player->m_vecOrigin();
			enemy_pos.z += 64.f;

			const auto angle = Math::CalcAngle(pLocal->GetEyePosition(), enemy_pos);
			const auto fov = Math::get_fov(original_viewangles, angle);

			if (fov < lowest_fov)
			{
				target = player;
				lowest_fov = fov;
				target_angle = angle;
			}
		}

		if (!target)
			return;

		cmd->viewangles.y = target_angle.y;
	}



}