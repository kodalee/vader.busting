#include "../Hooked.hpp"
#include "../../SDK/Displacement.hpp"
#include "../../SDK/sdk.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../SDK/Valve/CBaseHandle.hpp"
#include "../../Features/Rage/LagCompensation.hpp"
#include "../../Features/Rage/Resolver.hpp"
#include "../../Utils/address.h"
#include "../../Utils/stack.h"
#include "../../Utils/modules.h"
#include "../../Utils/pattern.h"


struct StackFrame
{
	StackFrame* Next;
	DWORD Return;
};

__forceinline DWORD GetReturnAddress(int depth = 0)
{
	StackFrame* fp;

	_asm mov fp, ebp;

	for (int i = 0; i < depth; i++)
	{
		if (!fp)
			break;

		fp = fp->Next;
	}

	return fp ? fp->Return : 0;
}

namespace Hooked
{
	void m_nSmokeEffectTickBegin(CRecvProxyData* pData, void* pStruct, void* pOut) {
		g_Vars.globals.szLastHookCalled = XorStr("28");

		if (!pData || !pStruct || !pOut)
			return;

		Interfaces::m_pDidSmokeEffectSwap->GetOriginalFunction()(pData, pStruct, pOut);
		if (g_Vars.esp.remove_smoke) {
			*reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(pOut) + 0x1) = true;
		}
	}

	//void __cdecl RecvProxy_m_flLowerBodyYawTarget(CRecvProxyData* data, void* ptr, void* out) {

	//	if (!data || !ptr || !out)
	//		return;

	//	g_Vars.globals.szLastHookCalled = XorStr("48");

	//	static DWORD fnCopyNewEntity = Memory::Scan(XorStr("engine.dll"), XorStr("EB 3F FF 77 10"));

	//	if (fnCopyNewEntity != GetReturnAddress(2))
	//		Engine::g_Resolver.on_lby_proxy((C_CSPlayer*)ptr, &data->m_Value.m_Float);

	//	// BALLS

	//	return Interfaces::m_Body_original->GetOriginalFunction()(data, ptr, out);
	//}

	void RecvProxy_m_flAbsYaw(CRecvProxyData* pData, void* pStruct, void* pOut) {
		g_Vars.globals.szLastHookCalled = XorStr("29");

		if (!pData || !pStruct || !pOut)
			return;

		Interfaces::m_pFlAbsYawSwap->GetOriginalFunction()(pData, pStruct, pOut);

		if (Interfaces::m_pEngine->IsConnected() && Interfaces::m_pEngine->IsInGame()) {
			CBaseHandle handle = *(CBaseHandle*)((uintptr_t)pStruct + Engine::Displacement.DT_CSRagdoll.m_hPlayer);
			if (handle.IsValid()) {
				auto player = (C_CSPlayer*)handle.Get();

				if (player) {
					auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->EntIndex()).Xor();
					if (lag_data && lag_data->m_History.size()) {
						lag_data->m_bGotAbsYaw = true;
						lag_data->m_flAbsYawHandled = pData->m_Value.m_Float;
					}
				}
			}

			Interfaces::m_pFlAbsYawSwap->GetOriginalFunction()(pData, pStruct, pOut);
		}
	}

	void m_bClientSideAnimation(CRecvProxyData* pData, void* pStruct, void* pOut) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return Interfaces::m_bClientSideAnimationSwap->GetOriginalFunction()(pData, pStruct, pOut);

		auto player = (C_CSPlayer*)pStruct;

		if (player && player->IsPlayer() && !player->IsTeammate(local))
			*(int*)pOut = (g_Vars.globals.m_bUpdatingAnimations ? 1 : 0);
	}

	void RecvProxy_PlaybackRate(CRecvProxyData* pData, void* pStruct, void* pOut) {
		g_Vars.globals.szLastHookCalled = XorStr("47");
		// PlaybackRate
		Interfaces::m_pPlaybackRateSwap->GetOriginalFunction()(pData, pStruct, pOut);

		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

		if (!LocalPlayer)
			return;

		auto pAnimOverlay = (C_AnimationLayer*)pStruct;
		if (pAnimOverlay) {
			auto player = (C_BasePlayer*)pAnimOverlay->m_pOwner;
			if (!player || player == LocalPlayer)
				return;

			auto& lag_data = Engine::LagCompensation::Get()->GetLagData(player->EntIndex());
			if (lag_data.Xor()) {
				lag_data->m_flRate = pAnimOverlay->m_flPlaybackRate;
				lag_data->m_bRateCheck = true;
			}
		}
	}

	void Body_proxy(CRecvProxyData* pData, void* pStruct, void* pOut) {
		g_Vars.globals.szLastHookCalled = XorStr("51");

			static DWORD RecvTable_Decode{ pattern::find(PE::GetModule(HASH("engine.dll")), XorStr("EB 0D FF 77 10")) };

			const auto player = reinterpret_cast<C_CSPlayer*>(pStruct);

			// call from entity going into pvs.
	        if (RecvTable_Decode != GetReturnAddress(2)) {
				//printf("faggot\n");

				// store data about the update.
				Engine::g_Resolver.OnBodyUpdate(player, pData->m_Value.m_Float);
			}

			// call original proxy.
			Interfaces::m_pFlAbsYawSwap->GetOriginalFunction()(pData, pStruct, pOut);

	}
}
