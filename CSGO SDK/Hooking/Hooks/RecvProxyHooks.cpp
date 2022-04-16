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

// shit below !!! dont look!!!!

namespace DetailFnv
{
	template <typename Type, Type OffsetBasis, Type Prime>
	struct size_dependant_data
	{
		using type = Type;
		constexpr static auto k_offset_basis = OffsetBasis;
		constexpr static auto k_prime = Prime;
	};

	template <size_t Bits>
	struct size_selector;

	template <>
	struct size_selector<32>
	{
		using type = size_dependant_data<std::uint32_t, 0x811c9dc5ul, 16777619ul>;
	};

	template <>
	struct size_selector<64>
	{
		using type = size_dependant_data<std::uint64_t, 0xcbf29ce484222325ull, 1099511628211ull>;
	};

	template <std::size_t Size>
	class fnv_hash
	{
	private:
		using data_t = typename size_selector<Size>::type;

	public:
		using hash = typename data_t::type;

	private:
		constexpr static auto k_offset_basis = data_t::k_offset_basis;
		constexpr static auto k_prime = data_t::k_prime;

	public:
		template <std::size_t N>
		static __forceinline constexpr auto hash_constexpr(const char(&str)[N], const std::size_t size = N) -> hash
		{
			return static_cast<hash>(1ull * (size == 1
				? (k_offset_basis ^ str[0])
				: (hash_constexpr(str, size - 1) ^ str[size - 1])) * k_prime);
		}

		static auto __forceinline hash_runtime(const char* str) -> hash
		{
			auto result = k_offset_basis;
			do
			{
				result ^= *str++;
				result *= k_prime;
			} while (*(str - 1) != '\0');

			return result;
		}
	};
}

using new_fnv = ::DetailFnv::fnv_hash<sizeof(void*) * 8>;

#define FNV(str) (std::integral_constant<new_fnv::hash, new_fnv::hash_constexpr(str)>::value)

static int get_new_animation(const new_fnv::hash model, const int sequence) {
	enum ESequence {
		SEQUENCE_DEFAULT_DRAW = 0,
		SEQUENCE_DEFAULT_IDLE1 = 1,
		SEQUENCE_DEFAULT_IDLE2 = 2,
		SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
		SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
		SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
		SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
		SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
		SEQUENCE_DEFAULT_LOOKAT01 = 12,
	};
	switch (model) {
	case FNV("models/weapons/v_knife_butterfly.mdl"): {
		switch (sequence) {
		case SEQUENCE_DEFAULT_DRAW: return RandomInt(0, 1);
		case SEQUENCE_DEFAULT_LOOKAT01: return RandomInt(13, 15);
		default: return sequence + 1;
		}
	}
	case FNV("models/weapons/v_knife_falchion_advanced.mdl"): {
		switch (sequence) {
		case SEQUENCE_DEFAULT_IDLE2: return 1;
		case SEQUENCE_DEFAULT_HEAVY_MISS1: return RandomInt(8, 9);
		case SEQUENCE_DEFAULT_LOOKAT01: return RandomInt(12, 13);
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1: return sequence;
		default: return sequence - 1;
		}
	}
	case FNV("models/weapons/v_knife_push.mdl"): {
		switch (sequence) {
		case SEQUENCE_DEFAULT_IDLE2: return 1;
		case SEQUENCE_DEFAULT_LIGHT_MISS1:
		case SEQUENCE_DEFAULT_LIGHT_MISS2: return RandomInt(2, 6);
		case SEQUENCE_DEFAULT_HEAVY_MISS1: return RandomInt(11, 12);
		case SEQUENCE_DEFAULT_HEAVY_HIT1:
		case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
		case SEQUENCE_DEFAULT_LOOKAT01: return sequence + 3;
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1: return sequence;
		default: return sequence + 2;
		}
	}
	case FNV("models/weapons/v_knife_survival_bowie.mdl"): {
		switch (sequence) {
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1: return sequence;
		case SEQUENCE_DEFAULT_IDLE2: return 1;
		default: return sequence - 1;
		}
	}
	case FNV("models/weapons/v_knife_ursus.mdl"): {
		switch (sequence) {
		case SEQUENCE_DEFAULT_DRAW: return RandomInt(0, 1);
		case SEQUENCE_DEFAULT_LOOKAT01: return RandomInt(13, 14);
		default: return sequence + 1;
		}
	}
	case FNV("models/weapons/v_knife_stiletto.mdl"): {
		switch (sequence) {
		case SEQUENCE_DEFAULT_LOOKAT01: return RandomInt(12, 13);
		}
	}
	case FNV("models/weapons/v_knife_widowmaker.mdl"): {
		switch (sequence) {
		case SEQUENCE_DEFAULT_LOOKAT01: return RandomInt(14, 15);
		}
	}
	default: return sequence;
	}
}

struct weapon_info_proxy {
	constexpr weapon_info_proxy(const char* model, const char* icon = nullptr, int animindex = -1) : model(model), icon(icon), animindex(animindex) {}
	const char* model;
	const char* icon;
	int animindex;
};

const weapon_info_proxy* GetWeaponInfo(int defindex) {
	const static std::map<int, weapon_info_proxy> Info = {
		{ WEAPON_KNIFE_CT,{ XorStr("models/weapons/v_knife_default_ct.mdl"), XorStr("knife_default_ct"), 2 } },
		{ WEAPON_KNIFE_T,{ XorStr("models/weapons/v_knife_default_t.mdl"), XorStr("knife_t"), 12 } },
		{ WEAPON_KNIFE_BAYONET,{ XorStr("models/weapons/v_knife_bayonet.mdl"), XorStr("bayonet"), 0 } },
		{ WEAPON_KNIFE_FLIP,{ XorStr("models/weapons/v_knife_flip.mdl"), XorStr("knife_flip"), 4 } },
		{ WEAPON_KNIFE_GUT,{ XorStr("models/weapons/v_knife_gut.mdl"), XorStr("knife_gut"), 5 } },
		{ WEAPON_KNIFE_KARAMBIT,{ XorStr("models/weapons/v_knife_karam.mdl"), XorStr("knife_karambit"), 7 } },
		{ WEAPON_KNIFE_M9_BAYONET,{ XorStr("models/weapons/v_knife_m9_bay.mdl"), XorStr("knife_m9_bayonet"), 8 } },
		{ WEAPON_KNIFE_HUNTSMAN,{ XorStr("models/weapons/v_knife_tactical.mdl"), XorStr("knife_tactical") } },
		{ WEAPON_KNIFE_FALCHION,{ XorStr("models/weapons/v_knife_falchion_advanced.mdl"), XorStr("knife_falchion"), 3 } },
		{ WEAPON_KNIFE_BOWIE,{ XorStr("models/weapons/v_knife_survival_bowie.mdl"), XorStr("knife_survival_bowie"), 11 } },
		{ WEAPON_KNIFE_BUTTERFLY,{ XorStr("models/weapons/v_knife_butterfly.mdl"), XorStr("knife_butterfly"), 1 } },
		{ WEAPON_KNIFE_SHADOW_DAGGERS,{ XorStr("models/weapons/v_knife_push.mdl"), XorStr("knife_push"), 9 } },

	};
	const auto entry = Info.find(defindex);
	return entry == end(Info) ? nullptr : &entry->second;
}

// shit above!!! dont look!!!!

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

	void sequence_remap(CRecvProxyData* data, C_BaseViewModel* view_model, C_CSPlayer* local) {

		if (!g_Vars.misc.enable_skins) return;
		const auto owner = Interfaces::m_pEntList->GetClientEntityFromHandle(view_model->m_hOwner());
		if (local && owner && owner == local) {
			auto weapon = view_model->m_hWeapon().Get();
			if (!weapon) return;
			const auto weapon_info = GetWeaponInfo(weapon->m_Item().m_iItemDefinitionIndex());
			if (!weapon_info) return;
			//Replace anim
			auto& m_sequence = data->m_Value.m_Int;
			m_sequence = get_new_animation(new_fnv::hash_runtime(weapon_info->model), m_sequence);
		}
	}

	void _cdecl sequence_proxy(CRecvProxyData* data_const, void* p_struct, void* p_out) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

		if (!LocalPlayer)
			return;

		const auto data = const_cast<CRecvProxyData*>(data_const);
		const auto view_model = static_cast<C_BaseViewModel*>(p_struct);
		if (view_model && data && LocalPlayer) sequence_remap(data, view_model, LocalPlayer);
		Interfaces::m_sequence_proxy->GetOriginalFunction()(data, p_struct, p_out);
	}
}
