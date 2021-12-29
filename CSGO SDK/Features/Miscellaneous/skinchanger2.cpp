#include "skinchanger2.h"
#include "../../SDK/Classes/PropManager.hpp"
#include <algorithm>
#include <memory.h>
#include "../../SDK/Valve/recv_swap.hpp"
#include <stdlib.h>
#include <unordered_map>
#include "KitParser.hpp"


//typedef void(*RecvVarProxyFn)(const CRecvProxyData* pData, void* pStruct, void* pOut);

#define SEQUENCE_DEFAULT_DRAW 0
#define SEQUENCE_DEFAULT_IDLE1 1
#define SEQUENCE_DEFAULT_IDLE2 2
#define SEQUENCE_DEFAULT_LIGHT_MISS1 3
#define SEQUENCE_DEFAULT_LIGHT_MISS2 4
#define SEQUENCE_DEFAULT_HEAVY_MISS1 9
#define SEQUENCE_DEFAULT_HEAVY_HIT1 10
#define SEQUENCE_DEFAULT_HEAVY_BACKSTAB 11
#define SEQUENCE_DEFAULT_LOOKAT01 12

#define SEQUENCE_BUTTERFLY_DRAW 0
#define SEQUENCE_BUTTERFLY_DRAW2 1
#define SEQUENCE_BUTTERFLY_LOOKAT01 13
#define SEQUENCE_BUTTERFLY_LOOKAT03 15

#define SEQUENCE_FALCHION_IDLE1 1
#define SEQUENCE_FALCHION_HEAVY_MISS1 8
#define SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP 9
#define SEQUENCE_FALCHION_LOOKAT01 12
#define SEQUENCE_FALCHION_LOOKAT02 13

#define SEQUENCE_DAGGERS_IDLE1 1
#define SEQUENCE_DAGGERS_LIGHT_MISS1 2
#define SEQUENCE_DAGGERS_LIGHT_MISS5 6
#define SEQUENCE_DAGGERS_HEAVY_MISS2 11
#define SEQUENCE_DAGGERS_HEAVY_MISS1 12

#define SEQUENCE_BOWIE_IDLE1 1

#define GET_INDEX Interfaces::m_pModelInfo->GetModelIndex

using namespace std;

inline int RandomSequence(int low, int high) {
	return (rand() % (high - low + 1) + low);
}

#define	LIFE_ALIVE 0

#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);

//RecvVarProxyFn fnSequenceProxyFn = nullptr;

//RecvVarProxyFn oRecvnModelIndex;

struct knifes {
	int iBayonet;
	int iButterfly;
	int iFlip;
	int iGut;
	int iKarambit;
	int iM9Bayonet;
	int iHuntsman;
	int iFalchion;
	int iDagger;
	int iBowie;
	int iGunGame;
};


/*void Hooked_RecvProxy_Viewmodel(CRecvProxyData* pData, void* pStruct, void* pOut)
{
	knifes knives;
	int default_t = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_default_t.mdl");
	int default_ct = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
	knives.iBayonet = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
	knives.iButterfly = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
	knives.iFlip = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_flip.mdl");
	knives.iGut = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_gut.mdl");
	knives.iKarambit = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_karam.mdl");
	knives.iM9Bayonet = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
	knives.iHuntsman = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_tactical.mdl");
	knives.iFalchion = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
	knives.iDagger = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_push.mdl");
	knives.iBowie = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
	knives.iGunGame = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_gg.mdl");

	int Navaja = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_gypsy_jackknife.mdl");
	int Stiletto = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_stiletto.mdl");
	int Ursus = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_ursus.mdl");
	int Talon = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_widowmaker.mdl");

	// Get local player (just to stop replacing spectators knifes)
	if ((vars.skins.enable) && csgo->local)
	{
		// If we are alive and holding a default knife(if we already have a knife don't worry about changing)
		if (csgo->local->isAlive() && (
			pData->m_Value.m_Int == default_t ||
			pData->m_Value.m_Int == default_ct ||
			pData->m_Value.m_Int == knives.iBayonet ||
			pData->m_Value.m_Int == knives.iFlip ||
			pData->m_Value.m_Int == knives.iGunGame ||
			pData->m_Value.m_Int == knives.iGut ||
			pData->m_Value.m_Int == knives.iKarambit ||
			pData->m_Value.m_Int == knives.iM9Bayonet ||
			pData->m_Value.m_Int == knives.iHuntsman ||
			pData->m_Value.m_Int == knives.iBowie ||
			pData->m_Value.m_Int == knives.iButterfly ||
			pData->m_Value.m_Int == knives.iFalchion ||
			pData->m_Value.m_Int == knives.iDagger ||
			pData->m_Value.m_Int == Navaja ||
			pData->m_Value.m_Int == Stiletto ||
			pData->m_Value.m_Int == Ursus ||
			pData->m_Value.m_Int == Talon))
		{
			// Set whatever knife we want
			if (vars.skins.knife_model == 0)
				pData->m_Value.m_Int = knives.iBayonet;
			else if (vars.skins.knife_model == 9)
				pData->m_Value.m_Int = knives.iBowie;
			else if (vars.skins.knife_model == 6)
				pData->m_Value.m_Int = knives.iButterfly;
			else if (vars.skins.knife_model == 7)
				pData->m_Value.m_Int = knives.iFalchion;
			else if (vars.skins.knife_model == 1)
				pData->m_Value.m_Int = knives.iFlip;
			else if (vars.skins.knife_model == 2)
				pData->m_Value.m_Int = knives.iGut;
			else if (vars.skins.knife_model == 5)
				pData->m_Value.m_Int = knives.iHuntsman;
			else if (vars.skins.knife_model == 3)
				pData->m_Value.m_Int = knives.iKarambit;
			else if (vars.skins.knife_model == 4)
				pData->m_Value.m_Int = knives.iM9Bayonet;
			else if (vars.skins.knife_model == 8)
				pData->m_Value.m_Int = knives.iDagger;
			else if (vars.skins.knife_model == 10)
				pData->m_Value.m_Int = Navaja;
			else if (vars.skins.knife_model == 11)
				pData->m_Value.m_Int = Stiletto;
			else if (vars.skins.knife_model == 12)
				pData->m_Value.m_Int = Ursus;
			else if (vars.skins.knife_model == 13)
				pData->m_Value.m_Int = Talon;
		}
	}	
	oRecvnModelIndex(pData, pStruct, pOut);
}*/

/*void SetViewModelSequence2(const CRecvProxyData* pDataConst, void* pStruct, void* pOut)
{
	CRecvProxyData* pData = const_cast<CRecvProxyData*>(pDataConst);

	// Confirm that we are replacing our view model and not someone elses.
	IBaseViewModel* pViewModel = (IBaseViewModel*)pStruct;

	if (pViewModel) {
		IBasePlayer* pOwner = interfaces.ent_list->GetClientEntityFromHandle(pViewModel->GetOwner());

		// Compare the owner entity of this view model to the local player entity.
		if (pOwner && pOwner->GetIndex() == interfaces.engine->GetLocalPlayer()) {
			// Get the filename of the current view model.
			void* pModel = interfaces.models.model_info->GetModel(pViewModel->GetModelIndex());
			const char* szModel = interfaces.models.model_info->GetModelName(pModel);

			// Store the current sequence.
			int m_nSequence = pData->m_Value.m_Int;

			if (!strcmp(szModel, "models/weapons/v_knife_butterfly.mdl")) {
				// Fix animations for the Butterfly Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_DRAW:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2); break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03); break;
				default:
					m_nSequence++;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_falchion_advanced.mdl")) {
				// Fix animations for the Falchion Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_FALCHION_IDLE1; break;
				case SEQUENCE_DEFAULT_HEAVY_MISS1:
					m_nSequence = RandomInt(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP); break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02); break;
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				default:
					m_nSequence--;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_push.mdl")) {
				// Fix animations for the Shadow Daggers.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_DAGGERS_IDLE1; break;
				case SEQUENCE_DEFAULT_LIGHT_MISS1:
				case SEQUENCE_DEFAULT_LIGHT_MISS2:
					m_nSequence = RandomInt(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5); break;
				case SEQUENCE_DEFAULT_HEAVY_MISS1:
					m_nSequence = RandomInt(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1); break;
				case SEQUENCE_DEFAULT_HEAVY_HIT1:
				case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence += 3; break;
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				default:
					m_nSequence += 2;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_survival_bowie.mdl")) {
				// Fix animations for the Bowie Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_BOWIE_IDLE1; break;
				default:
					m_nSequence--;
				}
			}

			// Set the fixed sequence.
			pData->m_Value.m_Int = m_nSequence;
		}
	}

	// Call original function with the modified data.
	fnSequenceProxyFn(pData, pStruct, pOut);
}*/

/*void AnimationFixHook()
{
	for (ClientClass* pClass = interfaces.client->GetAllClasses(); pClass; pClass = pClass->m_pNext) {
		if (!strcmp(pClass->m_pNetworkName, "CBaseViewModel")) {
			// Search for the 'm_nModelIndex' property.
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++) {
				RecvProp* pProp = &pClassTable->m_pProps[nIndex];

				if (!pProp || strcmp(pProp->m_pVarName, "m_nSequence"))
					continue;

				// Store the original proxy function.
				fnSequenceProxyFn = static_cast<RecvVarProxyFn>(pProp->m_ProxyFn);

				// Replace the proxy function with our sequence changer.
				pProp->m_ProxyFn = static_cast<RecvVarProxyFn>(SetViewModelSequence2);

				break;
			}

			break;
		}
	}
}*/
/*void skins_speedy::NetvarHook()
{
	AnimationFixHook();
	/*ClientClass* pClass = interfaces.client->GetAllClasses();
	while (pClass)
	{
		const char* pszName = pClass->m_pRecvTable->m_pNetTableName;
		if (!strcmp(pszName, "DT_BaseViewModel"))
		{
			for (int i = 0; i < pClass->m_pRecvTable->m_nProps; i++)
			{
				RecvProp* pProp = &(pClass->m_pRecvTable->m_pProps[i]);
				const char* name = pProp->m_pVarName;
				if (!strcmp(name, "m_nModelIndex"))
				{
					oRecvnModelIndex = (RecvVarProxyFn)pProp->m_ProxyFn;
					pProp->m_ProxyFn = (RecvVarProxyFn)Hooked_RecvProxy_Viewmodel;
				}
			}
		}
		pClass = pClass->m_pNext;
	}*/
//}

C_BaseEntity* pWeapon;
C_BaseEntity* worldmodel;
std::unordered_map<const char*, const char*> killIcons = {};

int item_def_knifes()
{
	switch (g_Vars.misc.knife_model)
	{
	case 0:
		return 500;
		break;
	case 1:
		return 514;
		break;
	case 2:
		return 515;
		break;
	case 3:
		return 512;
		break;
	case 4:
		return 505;
		break;
	case 5:
		return 506;
		break;
	case 6:
		return 509;
		break;
	case 7:
		return 507;
		break;
	case 8:
		return 508;
		break;
	case 9:
		return 516;
		break;
	case 10:
		return 520;
		break;
	case 11:
		return 522;
		break;
	case 12:
		return 519;
		break;
	case 13:
		return 523;
		break;
	case 14:
		return 503;
		break;
	case 15:
		return 525;
		break;
	case 16:
		return 521;
		break;
	case 17:
		return 518;
		break;
	case 18:
		return 517;
		break;
	default:
		break;
	}
}

struct weapon_info {
	constexpr weapon_info(const char* model, const char* icon = nullptr, int animindex = -1) : model(model), icon(icon), animindex(animindex) {}
	const char* model;
	const char* icon;
	int animindex;
};

const weapon_info* GetWeaponInfo(int defindex) {
	const static std::map<int, weapon_info> Info = {
		{ WEAPON_KNIFE_CT,{ "models/weapons/v_knife_default_ct.mdl", "knife_default_ct", 2 } },
		{ WEAPON_KNIFE_T,{ "models/weapons/v_knife_default_t.mdl", "knife_t", 12 } },
		{ WEAPON_KNIFE_BAYONET,{ "models/weapons/v_knife_bayonet.mdl", "bayonet", 0 } },
		{ WEAPON_KNIFE_FLIP,{ "models/weapons/v_knife_flip.mdl", "knife_flip", 4 } },
		{ WEAPON_KNIFE_GUT,{ "models/weapons/v_knife_gut.mdl", "knife_gut", 5 } },
		{ WEAPON_KNIFE_KARAMBIT,{ "models/weapons/v_knife_karam.mdl", "knife_karambit", 7 } },
		{ WEAPON_KNIFE_M9_BAYONET,{ "models/weapons/v_knife_m9_bay.mdl", "knife_m9_bayonet", 8 } },
		{ WEAPON_KNIFE_HUNTSMAN,{ "models/weapons/v_knife_tactical.mdl", "knife_tactical" } },
		{ WEAPON_KNIFE_FALCHION,{ "models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion", 3 } },
		{ WEAPON_KNIFE_BOWIE,{ "models/weapons/v_knife_survival_bowie.mdl", "knife_survival_bowie", 11 } },
		{ WEAPON_KNIFE_BUTTERFLY,{ "models/weapons/v_knife_butterfly.mdl", "knife_butterfly", 1 } },
		{ WEAPON_KNIFE_SHADOW_DAGGERS,{ "models/weapons/v_knife_push.mdl", "knife_push", 9 } },

	};
	const auto entry = Info.find(defindex);
	return entry == end(Info) ? nullptr : &entry->second;
}

static auto get_wearable_create_fn() -> CreateClientClassFn {
	auto client_class = Interfaces::m_pClient->GetAllClasses();
	for (client_class = Interfaces::m_pClient->GetAllClasses();
		client_class; client_class = client_class->m_pNext) {
		if (client_class->m_ClassID == CEconWearable)
			return (CreateClientClassFn)client_class->m_pCreateFn;
	}
}

bool apply_skin(C_CSPlayer* local, C_BaseAttributableItem* skin, const char* model, int item_definition_index, int paint_kit, int model_index, int entity_quality, float FallbackWear) {
	skin->m_Item().m_iItemDefinitionIndex() = item_definition_index;
	skin->m_Item().m_nFallbackPaintKit() = paint_kit;
	skin->m_Item().m_iEntityQuality() = entity_quality;
	skin->m_Item().m_flFallbackWear() = FallbackWear;
	skin->m_nModelIndex() = model_index;
	//skin->ViewModelIndex() = model_index;
	HANDLE worldmodel_handle2 = skin->m_hWeaponWorldModel().Get();
	if (worldmodel_handle2) {
		worldmodel = (C_BaseAttributableItem*)Interfaces::m_pEntList->GetClientEntityFromHandle(skin->m_hWeaponWorldModel());
	}
	if (worldmodel) {
		worldmodel->m_nModelIndex() = model_index + 1;
	}
	auto local_player = reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntity(Interfaces::m_pEngine->GetLocalPlayer()));
	if (!local_player) {
		return false;
	}

	auto viewmodel = (C_BaseViewModel*)local->m_hViewModel().Get();

	if (!viewmodel)
		return false;

	static int offset = Engine::PropManager::Instance()->GetOffset(XorStr("DT_BaseViewModel"), XorStr("m_hWeapon"));
	auto m_hWeapon = *(CBaseHandle*)(uintptr_t(viewmodel) + offset);

	auto view_model_weapon = (C_BaseViewModel*)(Interfaces::m_pEntList->GetClientEntityFromHandle(m_hWeapon));

	if (view_model_weapon != skin)
		return false;

	viewmodel->m_nModelIndex() = Interfaces::m_pModelInfo->GetModelIndex(model);
	return true;
}

void skins_speedy::Skinchanger()
{
	auto model_bayonet = "models/weapons/v_knife_bayonet.mdl";
	auto model_m9 = "models/weapons/v_knife_m9_bay.mdl";
	auto model_karambit = "models/weapons/v_knife_karam.mdl";
	auto model_bowie = "models/weapons/v_knife_survival_bowie.mdl";
	auto model_butterfly = "models/weapons/v_knife_butterfly.mdl";
	auto model_falchion = "models/weapons/v_knife_falchion_advanced.mdl";
	auto model_flip = "models/weapons/v_knife_flip.mdl";
	auto model_gut = "models/weapons/v_knife_gut.mdl";
	auto model_huntsman = "models/weapons/v_knife_tactical.mdl";
	auto model_daggers = "models/weapons/v_knife_push.mdl";

	if (!Interfaces::m_pEngine->IsConnected() && !Interfaces::m_pEngine->IsInGame()) {
		return;
	}

	auto local = C_CSPlayer::GetLocalPlayer();
	if (!local)
		return;

	auto local_player = reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntity(Interfaces::m_pEngine->GetLocalPlayer()));
	if (!local_player) {
		return;
	}

	if (g_Vars.misc.enable_skins) {
		auto active_weapon = local_player->m_hActiveWeapon().Get();
		C_WeaponCSBaseGun* pWeapon = (C_WeaponCSBaseGun*)local->m_hActiveWeapon().Get();

		if (!active_weapon || !pWeapon || pWeapon->m_iItemDefinitionIndex() == WEAPON_ZEUS) {
			return;
		}
		auto my_weapons = local_player->m_hMyWeapons();

		for (int i = 0; i < 48; ++i) {
			auto weapon = (C_BaseAttributableItem*)my_weapons[i].Get();

			if (!weapon) {
				continue;
			}

			auto wear = 0.001f;

			if (weapon->IsKnife()) {
				switch (g_Vars.misc.knife_model) {
				case 0:
					apply_skin(local, weapon, model_bayonet, WEAPON_KNIFE_BAYONET, g_Vars.misc.knife_skin, GET_INDEX(model_bayonet), 3, wear);
					break;
				case 1:
					apply_skin(local, weapon, model_bowie, WEAPON_KNIFE_BOWIE, g_Vars.misc.knife_skin, GET_INDEX(model_bowie), 3, wear);
					break;
				case 2:
					apply_skin(local, weapon, model_butterfly, WEAPON_KNIFE_BUTTERFLY, g_Vars.misc.knife_skin, GET_INDEX(model_butterfly), 3, wear);
					break;
				case 3:
					apply_skin(local, weapon, model_falchion, WEAPON_KNIFE_FALCHION, g_Vars.misc.knife_skin, GET_INDEX(model_falchion), 3, wear);
					break;
				case 4:
					apply_skin(local, weapon, model_flip, WEAPON_KNIFE_FLIP, g_Vars.misc.knife_skin, GET_INDEX(model_flip), 3, wear);
					break;
				case 5:
					apply_skin(local, weapon, model_gut, WEAPON_KNIFE_GUT, g_Vars.misc.knife_skin, GET_INDEX(model_gut), 3, wear);
					break;
				case 6:
					apply_skin(local, weapon, model_huntsman, WEAPON_KNIFE_HUNTSMAN, g_Vars.misc.knife_skin, GET_INDEX(model_huntsman), 3, wear);
					break;
				case 7:
					apply_skin(local, weapon, model_karambit, WEAPON_KNIFE_KARAMBIT, g_Vars.misc.knife_skin, GET_INDEX(model_karambit), 3, wear);
					break;
				case 8:
					apply_skin(local, weapon, model_m9, WEAPON_KNIFE_M9_BAYONET, g_Vars.misc.knife_skin, GET_INDEX(model_m9), 3, wear);
					break;
				case 9:
					apply_skin(local, weapon, model_daggers, WEAPON_KNIFE_SHADOW_DAGGERS, g_Vars.misc.knife_skin, GET_INDEX(model_daggers), 3, wear);
					break;
				}
			}

			switch (weapon->m_Item().m_iItemDefinitionIndex()) {
			case WEAPON_USPS:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.usp_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_P2000:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.p2k_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_GLOCK:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.glock_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_P250:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.p250_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_FIVESEVEN:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.fiveseven_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_TEC9:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.tec9_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_CZ75A:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.cz75a_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_ELITE:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.elite_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_DEAGLE:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.deagle_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_REVOLVER:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.revolver_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_FAMAS:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.famas_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_GALIL:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.galilar_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_M4A4:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.m4a1_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_M4A1S:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.m4a1s_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_AK47:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.ak47_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_SG553:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.sg556_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_AUG:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.aug_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_SSG08:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.ssg08_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_AWP:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.awp_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_SCAR20:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.scar20_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_G3SG1:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.g3sg1_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_SAWEDOFF:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.sawedoff_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_M249:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.m249_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_NEGEV:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.negev_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_MAG7:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.mag7_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_XM1014:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.xm1014_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_NOVA:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.nova_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_BIZON:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.bizon_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_MP7:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.mp7_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_MP9:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.mp9_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_MAC10:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.mac10_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_P90:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.p90_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_UMP45:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.ump45_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			}
			weapon->m_OriginalOwnerXuidLow() = 0;
			weapon->m_OriginalOwnerXuidHigh() = 0;
			weapon->m_nFallbackSeed() = 661;
			weapon->m_Item().m_iItemIDHigh() = -1;
		}
	}
}