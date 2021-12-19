#include "SkinChanger.hpp"
#include "../../SDK/CVariables.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../../SDK/Valve/CBaseHandle.hpp"
#include "KitParser.hpp"
#include "../../SDK/Classes/PropManager.hpp"
#include <algorithm>
#include <memory.h>
#include "../../SDK/Valve/recv_swap.hpp"
#include "../../Utils/FnvHash.hpp"
#include "../../SDK/displacement.hpp"
#include "../../Utils/hud.h"

static auto is_knife( const int i ) -> bool {
	return ( i >= WEAPON_KNIFE_BAYONET && i < GLOVE_STUDDED_BLOODHOUND ) || i == WEAPON_KNIFE_T || i == WEAPON_KNIFE_CT;
}

void Skins::load() {
	// update model indexes on mapload.
	m_knife_data[knives_t::BAYONET].setup(KNIFE_BAYONET,
		XorStr("models/weapons/v_knife_bayonet.mdl"),
		XorStr("models/weapons/w_knife_bayonet.mdl"),
		XorStr("bayonet"));

	m_knife_data[knives_t::BOWIE].setup(KNIFE_BOWIE,
		XorStr("models/weapons/v_knife_survival_bowie.mdl"),
		XorStr("models/weapons/w_knife_survival_bowie.mdl"),
		XorStr("knife_survival_bowie"));


	m_knife_data[knives_t::BUTTERFLY].setup(KNIFE_BUTTERFLY,
		XorStr("models/weapons/v_knife_butterfly.mdl"),
		XorStr("models/weapons/w_knife_butterfly.mdl"),
		XorStr("knife_butterfly"));

	m_knife_data[knives_t::FALCHION].setup(KNIFE_FALCHION,
		XorStr("models/weapons/v_knife_falchion_advanced.mdl"),
		XorStr("models/weapons/w_knife_falchion_advanced.mdl"),
		XorStr("knife_falchion"));

	m_knife_data[knives_t::FLIP].setup(KNIFE_FLIP,
		XorStr("models/weapons/v_knife_flip.mdl"),
		XorStr("models/weapons/w_knife_flip.mdl"),
		XorStr("knife_flip"));

	m_knife_data[knives_t::GUT].setup(KNIFE_GUT,
		XorStr("models/weapons/v_knife_gut.mdl"),
		XorStr("models/weapons/w_knife_gut.mdl"),
		XorStr("knife_gut"));

	m_knife_data[knives_t::HUNTSMAN].setup(KNIFE_HUNTSMAN,
		XorStr("models/weapons/v_knife_tactical.mdl"),
		XorStr("models/weapons/w_knife_tactical.mdl"),
		XorStr("knife_tactical"));

	m_knife_data[knives_t::KARAMBIT].setup(KNIFE_KARAMBIT,
		XorStr("models/weapons/v_knife_karam.mdl"),
		XorStr("models/weapons/w_knife_karam.mdl"),
		XorStr("knife_karambit"));

	m_knife_data[knives_t::M9].setup(KNIFE_M9_BAYONET,
		XorStr("models/weapons/v_knife_m9_bay.mdl"),
		XorStr("models/weapons/w_knife_m9_bay.mdl"),
		XorStr("knife_m9_bayonet"));

	m_knife_data[knives_t::DAGGER].setup(KNIFE_SHADOW_DAGGERS,
		XorStr("models/weapons/v_knife_push.mdl"),
		XorStr("models/weapons/w_knife_push.mdl"),
		XorStr("knife_push"));

	// update glove model indexes on mapload.
	m_glove_data[gloves_t::BLOODHOUND].setup(5027,
		XorStr("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"),
		XorStr("models/weapons/w_models/arms/glove_bloodhound/w_glove_bloodhound.mdl"));

	m_glove_data[gloves_t::SPORTY].setup(5030,
		XorStr("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"),
		XorStr("models/weapons/w_models/arms/glove_sporty/w_glove_sporty.mdl"));

	m_glove_data[gloves_t::DRIVER].setup(5031,
		XorStr("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"),
		XorStr("models/weapons/w_models/arms/glove_slick/w_glove_slick.mdl"));

	m_glove_data[gloves_t::HANDWRAP].setup(5032,
		XorStr("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"),
		XorStr("models/weapons/w_models/arms/glove_handwrap_leathery/w_glove_handwrap_leathery.mdl"));

	m_glove_data[gloves_t::MOTO].setup(5033,
		XorStr("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"),
		XorStr("models/weapons/w_models/arms/glove_motorcycle/w_glove_motorcycle.mdl"));

	m_glove_data[gloves_t::SPECIALIST].setup(5034,
		XorStr("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"),
		XorStr("models/weapons/w_models/arms/glove_specialist/w_glove_specialist.mdl"));

	m_update_time = 0.f;
}

void Skins::think() {
	std::vector< C_WeaponCSBaseGun* > weapons{};

	auto local = C_CSPlayer::GetLocalPlayer();
	if (!local || !local->IsAlive())
		return;

	if (!g_Vars.m_global_skin_changer.enable)
		return;

	if (!Interfaces::m_pEngine->IsInGame())
		return;

	player_info_t info;
	if (!Interfaces::m_pEngine->GetPlayerInfo(local->EntIndex(), &info))
		return;

	// store knife index.
	KnifeData* knife = &m_knife_data[g_Vars.m_global_skin_changer.knife];

	for (int i{ 1 }; i <= Interfaces::m_pEntList->GetHighestEntityIndex(); ++i) {
		C_CSPlayer* ent = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(i);
		if (!ent)
			continue;

		// run knifechanger.
		if (g_Vars.m_global_skin_changer.knife > 0 && ent->is("CPredictedViewModel")) {
			// get weapon entity from predicted viewmodel.
			auto weapon = (C_WeaponCSBaseGun*)ent->m_hActiveWeapon().Get();
			if (!weapon) {
				continue;
			}

			auto weaponInfo = weapon->GetCSWeaponData();
			if (!weaponInfo.IsValid()) {
				continue;
			}

			auto weapon2 = (C_BaseAttributableItem*)ent->m_hActiveWeapon().Get();
			if (!weapon2)
				continue;
			//auto weaponWorldModel = weapon2 ? (C_CSPlayer*)(weapon2)->m_hWeaponWorldModel().Get() : nullptr;

			if ((weapon->m_iItemDefinitionIndex() >= WEAPON_KNIFE_BAYONET && weapon->m_iItemDefinitionIndex() < GLOVE_STUDDED_BLOODHOUND) ||
				weapon->m_iItemDefinitionIndex() == WEAPON_KNIFE_T || weapon->m_iItemDefinitionIndex() == WEAPON_KNIFE_CT)
				continue;


			// this weapon does not belong to us, we are done here.
			// should barely happen unless you pick up someones knife.
			// possible on servers with 'mp_drop_knife_enable 1'.
			if (info.xuid_low != weapon2->m_OriginalOwnerXuidLow() || info.xuid_high != weapon2->m_OriginalOwnerXuidHigh())
				continue;

			// get and validate world model handle from the weapon
			//Weapon* weapon_world_model = weapon->GetWeaponWorldModel();
			auto weapon_world_model = weapon2 ? (C_CSPlayer*)(weapon2)->m_hWeaponWorldModel().Get() : nullptr;
			if (!weapon_world_model)
				continue;

			// set model index of the predicted viewmodel.
			ent->m_nModelIndex() = knife->m_model_index;

			// set the world model index.
			// do this to have knifechanger in third person. verry p.
			weapon_world_model->m_nModelIndex() = knife->m_world_model_index;

			// correct m_nSequence and m_flCycle.
			UpdateAnimations(ent);
		}

		else if (ent->IsBaseCombatWeapon()) {
			// cast to weapon class.
			auto weapon = (C_WeaponCSBaseGun*)ent->m_hActiveWeapon().Get();
			if (!weapon) {
				continue;
			}

			auto weaponInfo = weapon->GetCSWeaponData();
			if (!weaponInfo.IsValid()) {
				continue;
			}

			auto weapon2 = (C_BaseAttributableItem*)ent->m_hActiveWeapon().Get();
			if (!weapon2)
				continue;

			// this is not our gun.
			if (info.xuid_low != weapon2->m_OriginalOwnerXuidLow() || info.xuid_high != weapon2->m_OriginalOwnerXuidHigh())
				continue;

			// we found a weapon that we own.
			weapons.push_back(weapon);

			if ((weapon->m_iItemDefinitionIndex() >= WEAPON_KNIFE_BAYONET && weapon->m_iItemDefinitionIndex() < GLOVE_STUDDED_BLOODHOUND) ||
				weapon->m_iItemDefinitionIndex() == WEAPON_KNIFE_T || weapon->m_iItemDefinitionIndex() == WEAPON_KNIFE_CT) {
				// if this weapon is a knife, set some additional stuff.
				if (g_Vars.m_global_skin_changer.knife > 0) {
					// set the item id, this is for the hud.
					weapon->m_iItemDefinitionIndex() = knife->m_id;

					// not needed. but everyone does this, try without.
					weapon->m_nModelIndex() = knife->m_model_index;

					// not needed. but everyone does this, try without.
					weapon->m_iViewModelIndex() = knife->m_model_index;

					// not needed. but everyone does this, try without.
					weapon->m_iWorldModelIndex() = knife->m_world_model_index;

					// set the little star thing.
					weapon->m_Item().m_iEntityQuality() = 3;
				}

				else if (g_Vars.m_global_skin_changer.knife == 0) {
					// fix definition.
					if (local->m_iTeamNum() == 2)
						weapon->m_iItemDefinitionIndex() = KNIFE_T;

					else if (local->m_iTeamNum() == 3)
						weapon->m_iItemDefinitionIndex() = KNIFE_CT;

					// reset.
					weapon->m_Item().m_iEntityQuality() = 0;
					weapon->m_Item().m_nFallbackPaintKit() = -1;
					weapon->m_Item().m_nFallbackStatTrak() = -1;
				}
			}

			switch (weapon->m_iItemDefinitionIndex()) {
			case DEAGLE:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_deagle;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_deagle ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_deagle;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_deagle) / 100.f) + FLT_EPSILON;
				break;
			case ELITE:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_elite;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_elite ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_elite;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_elite) / 100.f) + FLT_EPSILON;
				break;
			case FIVESEVEN:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_fiveseven;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_fiveseven ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_fiveseven;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_fiveseven) / 100.f) + FLT_EPSILON;
				break;
			case GLOCK:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_glock;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_glock ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_glock;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_glock) / 100.f) + FLT_EPSILON;
				break;
			case AK47:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_ak47;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_ak47 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_ak47;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_ak47) / 100.f) + FLT_EPSILON;
				break;
			case AUG:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_aug;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_aug ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_aug;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_aug) / 100.f) + FLT_EPSILON;
				break;
			case AWP:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_awp;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_awp ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_awp;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_awp) / 100.f) + FLT_EPSILON;
				break;
			case FAMAS:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_famas;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_famas ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_famas;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_famas) / 100.f) + FLT_EPSILON;
				break;
			case G3SG1:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_g3sg1;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_g3sg1 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_g3sg1;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_g3sg1) / 100.f) + FLT_EPSILON;
				break;
			case GALIL:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_galil;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_galil ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_galil;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_galil) / 100.f) + FLT_EPSILON;
				break;
			case M249:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_m249;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_m249 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_m249;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_m249) / 100.f) + FLT_EPSILON;
				break;
			case M4A4:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_m4a4;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_m4a4 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_m4a4;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_m4a4) / 100.f) + FLT_EPSILON;
				break;
			case MAC10:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_mac10;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_mac10 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_mac10;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_mac10) / 100.f) + FLT_EPSILON;
				break;
			case P90:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_p90;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_p90 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_p90;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_p90) / 100.f) + FLT_EPSILON;
				break;
			case UMP45:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_ump45;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_ump45 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_ump45;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_ump45) / 100.f) + FLT_EPSILON;
				break;
			case XM1014:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_xm1014;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_xm1014 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_xm1014;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_xm1014) / 100.f) + FLT_EPSILON;
				break;
			case BIZON:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_bizon;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_bizon ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_bizon;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_bizon) / 100.f) + FLT_EPSILON;
				break;
			case MAG7:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_mag7;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_mag7 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_mag7;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_mag7) / 100.f) + FLT_EPSILON;
				break;
			case NEGEV:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_negev;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_negev ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_negev;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_negev) / 100.f) + FLT_EPSILON;
				break;
			case SAWEDOFF:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_sawedoff;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_sawedoff ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_sawedoff;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_sawedoff) / 100.f) + FLT_EPSILON;
				break;
			case TEC9:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_tec9;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_tec9 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_tec9;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_tec9) / 100.f) + FLT_EPSILON;
				break;
			case P2000:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_p2000;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_p2000 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_p2000;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_p2000) / 100.f) + FLT_EPSILON;
				break;
			case MP7:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_mp7;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_mp7 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_mp7;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_mp7) / 100.f) + FLT_EPSILON;
				break;
			case MP9:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_mp9;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_mp9 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_mp9;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_mp9) / 100.f) + FLT_EPSILON;
				break;
			case NOVA:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_nova;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_nova ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_nova;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_nova) / 100.f) + FLT_EPSILON;
				break;
			case P250:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_p250;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_p250 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_p250;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_p250) / 100.f) + FLT_EPSILON;
				break;
			case SCAR20:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_scar20;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_scar20 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_scar20;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_scar20) / 100.f) + FLT_EPSILON;
				break;
			case SG553:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_sg553;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_sg553 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_sg553;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_sg553) / 100.f) + FLT_EPSILON;
				break;
			case SSG08:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_ssg08;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_ssg08 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_ssg08;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_ssg08) / 100.f) + FLT_EPSILON;
				break;
			case M4A1S:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_m4a1s;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_m4a1s ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_m4a1s;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_m4a1s) / 100.f) + FLT_EPSILON;
				break;
			case USPS:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_usps;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_usps ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_usps;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_usps) / 100.f) + FLT_EPSILON;
				break;
			case CZ75A:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_cz75a;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_cz75a ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_cz75a;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_cz75a) / 100.f) + FLT_EPSILON;
				break;
			case REVOLVER:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_revolver;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_revolver ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_revolver;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_revolver) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_BAYONET:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_bayonet;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_bayonet ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_bayonet;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_bayonet) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_FLIP:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_flip;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_flip ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_flip;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_flip) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_GUT:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_gut;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_gut ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_gut;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_gut) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_KARAMBIT:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_karambit;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_karambit ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_karambit;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_karambit) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_M9_BAYONET:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_m9;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_m9 ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_m9;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_m9) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_HUNTSMAN:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_huntsman;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_huntsman ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_huntsman;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_huntsman) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_FALCHION:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_falchion;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_falchion ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_falchion;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_falchion) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_BOWIE:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_bowie;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_bowie ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_bowie;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_bowie) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_BUTTERFLY:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_butterfly;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_butterfly ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_butterfly;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_butterfly) / 100.f) + FLT_EPSILON;
				break;
			case KNIFE_SHADOW_DAGGERS:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.m_global_skin_changer.id_daggers;
				weapon->m_Item().m_nFallbackStatTrak() = g_Vars.m_global_skin_changer.stattrak_daggers ? 1337 : -1;
				weapon->m_Item().m_nFallbackSeed() = g_Vars.m_global_skin_changer.seed_daggers;
				weapon->m_Item().m_flFallbackWear() = ((100.f - g_Vars.m_global_skin_changer.quality_daggers) / 100.f) + FLT_EPSILON;
				break;

			default:
				break;
			}

			// fix stattrak ownership.
			weapon->m_Item().m_iAccountID() = info.xuid_low;

			// fix stattrak in hud.
			if (weapon->m_Item().m_nFallbackStatTrak() >= 0)
				weapon->m_Item().m_iEntityQuality() = 9;

			// force use fallback values.
			weapon->m_Item().m_iItemIDHigh() = -1;
		}
	}

	// only force update every 1s.
	if (m_update && Interfaces::m_pGlobalVars->curtime >= m_update_time) {
		for (auto& w : weapons)
			UpdateItem(w);

		m_update = false;
		m_update_time = Interfaces::m_pGlobalVars->curtime + 1.f;
	}
}

uintptr_t* rel32(uintptr_t ptr) {
	auto offset = *(uintptr_t*)(ptr + 0x1);
	return (uintptr_t*)(ptr + 5 + offset);
}

void Skins::UpdateItem(C_WeaponCSBaseGun* item) {
	if (!item || !item->IsBaseCombatWeapon())
		return;

	if (Interfaces::m_pClientState->m_nDeltaTick() == -1)
		return;

	//  if ( sub_106E32D0(v4, "round_start") )
	//		SFWeaponSelection::ShowAndUpdateSelection( v2, 2, 0, v10 );

	// v1 = CHud::FindElement(&g_HUD, "SFWeaponSelection");
	// if( v1 )
	//		SFWeaponSelection::ShowAndUpdateSelection( ( int * )v1, 2, 0, this );

	item->m_bCustomMaterialInitialized() = item->m_Item().m_nFallbackPaintKit() <= 0;

	C_EconItemView* view = &item->m_Item();

	item->m_CustomMaterials().RemoveAll();
	view->m_CustomMaterials().RemoveAll();

	size_t count = item->m_Item().m_VisualsDataProcessors().Count();
	for (size_t i{}; i < count; ++i) {
		auto& elem = item->m_Item().m_VisualsDataProcessors()[i];
		if (elem) {
			elem->unreference();
			elem = nullptr;
		}
	}

	item->m_Item().m_VisualsDataProcessors().RemoveAll();

	item->GetClientNetworkable()->PostDataUpdate(0);
	item->GetClientNetworkable()->OnDataChanged(0);

	using ShowAndUpdateSelection_t = void(__thiscall*)(CHudElement*, int, C_WeaponCSBaseGun*, bool);

	auto ShowAndUpdateSelection = (ShowAndUpdateSelection_t)rel32(Memory::Scan(XorStr("client.dll"), XorStr("E8 ? ? ? ? A1 ? ? ? ? F3 0F 10 40 ? C6 83")));

	auto SFWeaponSelection = CHud::Instance()->FindElement(fnv::hashRuntime("SFWeaponSelection"));
	ShowAndUpdateSelection(SFWeaponSelection, 0, item, false);
}

void Skins::UpdateAnimations(C_CSPlayer* ent) {
	int knife = g_Vars.m_global_skin_changer.knife;

	int seq = ent->m_nSequence();

	// check if this knife needs extra fixing.
	if (knife == knives_t::BUTTERFLY || knife == knives_t::FALCHION || knife == knives_t::DAGGER || knife == knives_t::BOWIE) {

		// fix the idle sequences.
		if (seq == sequence_default_idle1 || seq == sequence_default_idle2) {
			// set the animation to be completed.
			ent->m_flCycle() == 0.999f;

			// cycle change, re-render.
			ent->InvalidatePhysicsRecursive(0);
		}
	}

	auto random_sequence = [](const int low, const int high) -> int {
		return rand() % (high - low + 1) + low;
	};

	// fix sequences.
	if (m_last_seq != seq) {
		if (knife == knives_t::BUTTERFLY) {
			switch (seq) {
			case sequence_default_draw:
				seq = random_sequence(sequence_butterfly_draw, sequence_butterfly_draw2);
				break;

			case sequence_default_lookat01:
				seq = random_sequence(sequence_butterfly_lookat01, sequence_butterfly_lookat03);
				break;

			default:
				seq++;
				break;
			}
		}

		else if (knife == knives_t::FALCHION) {
			switch (seq) {
			case sequence_default_draw:
			case sequence_default_idle1:
				break;

			case sequence_default_idle2:
				seq = sequence_falchion_idle1;
				break;

			case sequence_default_heavy_miss1:
				seq = random_sequence(sequence_falchion_heavy_miss1, sequence_falchion_heavy_miss1_noflip);
				break;

			case sequence_default_lookat01:
				seq = random_sequence(sequence_falchion_lookat01, sequence_falchion_lookat02);
				break;
			}
		}

		else if (knife == knives_t::DAGGER) {
			switch (seq) {
			case sequence_default_idle2:
				seq = sequence_push_idle1;
				break;

			case sequence_default_heavy_hit1:
			case sequence_default_heavy_backstab:
			case sequence_default_lookat01:
				seq += 3;
				break;

			case sequence_default_heavy_miss1:
				seq = sequence_push_heavy_miss2;
				break;
			}

		}

		else if (knife == knives_t::BOWIE) {
			if (seq > sequence_default_idle1)
				seq--;
		}

		m_last_seq = seq;
	}

	// write back fixed sequence.
	ent->m_nSequence() == seq;
}
