#pragma once
#include "../../SDK/sdk.hpp"

constexpr int sequence_default_draw = 0;
constexpr int sequence_default_idle1 = 1;
constexpr int sequence_default_idle2 = 2;
constexpr int sequence_default_heavy_miss1 = 9;
constexpr int sequence_default_heavy_hit1 = 10;
constexpr int sequence_default_heavy_backstab = 11;
constexpr int sequence_default_lookat01 = 12;

constexpr int sequence_butterfly_draw = 0;
constexpr int sequence_butterfly_draw2 = 1;
constexpr int sequence_butterfly_lookat01 = 13;
constexpr int sequence_butterfly_lookat03 = 15;

constexpr int sequence_falchion_idle1 = 1;
constexpr int sequence_falchion_heavy_miss1 = 8;
constexpr int sequence_falchion_heavy_miss1_noflip = 9;
constexpr int sequence_falchion_lookat01 = 12;
constexpr int sequence_falchion_lookat02 = 13;

constexpr int sequence_push_idle1 = 1;
constexpr int sequence_push_heavy_miss2 = 11;

struct KnifeData {
	int         m_id;
	std::string m_model, m_world_model, m_name;
	int         m_model_index, m_world_model_index;

	__forceinline KnifeData() : m_id{}, m_model{}, m_world_model{}, m_name{}, m_model_index{}, m_world_model_index{} {}

	__forceinline void setup(int id, const std::string& model, const std::string& world_model, const std::string& name) {
		m_id = id;
		m_model = model;
		m_name = name;
		m_model_index = Interfaces::m_pModelInfo->GetModelIndex(model.data());
		m_world_model_index = Interfaces::m_pModelInfo->GetModelIndex(world_model.data());
	}
};

struct GloveData {
	int         m_id;
	std::string m_model, m_world_model;
	int         m_model_index, m_world_model_index;

	__forceinline GloveData() : m_id{}, m_model{}, m_model_index{} {}

	__forceinline void setup(int id, const std::string& model, const std::string& world_model) {
		m_id = id;
		m_model = model;
		m_world_model = world_model;
		m_model_index = Interfaces::m_pModelInfo->GetModelIndex(model.data());
		m_world_model_index = Interfaces::m_pModelInfo->GetModelIndex(world_model.data());
	}
};

class Skins {
public:
	enum knives_t {
		BAYONET = 1,
		BOWIE,
		BUTTERFLY,
		FALCHION,
		FLIP,
		GUT,
		HUNTSMAN,
		KARAMBIT,
		M9,
		DAGGER,
		KNIFE_MAX
	};

	enum gloves_t {
		BLOODHOUND = 1,
		SPORTY,
		DRIVER,
		HANDWRAP,
		MOTO,
		SPECIALIST,
		GLOVE_MAX
	};

	enum Weapons_t : int {
		DEAGLE = 1,
		ELITE = 2,
		FIVESEVEN = 3,
		GLOCK = 4,
		AK47 = 7,
		AUG = 8,
		AWP = 9,
		FAMAS = 10,
		G3SG1 = 11,
		GALIL = 13,
		M249 = 14,
		M4A4 = 16,
		MAC10 = 17,
		P90 = 19,
		UMP45 = 24,
		XM1014 = 25,
		BIZON = 26,
		MAG7 = 27,
		NEGEV = 28,
		SAWEDOFF = 29,
		TEC9 = 30,
		ZEUS = 31,
		P2000 = 32,
		MP7 = 33,
		MP9 = 34,
		NOVA = 35,
		P250 = 36,
		SCAR20 = 38,
		SG553 = 39,
		SSG08 = 40,
		KNIFE_T = 42,
		FLASHBANG = 43,
		HEGRENADE = 44,
		SMOKE = 45,
		MOLOTOV = 46,
		DECOY = 47,
		FIREBOMB = 48,
		C4 = 49,
		MUSICKIT = 58,
		KNIFE_CT = 59,
		M4A1S = 60,
		USPS = 61,
		TRADEUPCONTRACT = 62,
		CZ75A = 63,
		REVOLVER = 64,
		KNIFE_BAYONET = 500,
		KNIFE_FLIP = 505,
		KNIFE_GUT = 506,
		KNIFE_KARAMBIT = 507,
		KNIFE_M9_BAYONET = 508,
		KNIFE_HUNTSMAN = 509,
		KNIFE_FALCHION = 512,
		KNIFE_BOWIE = 514,
		KNIFE_BUTTERFLY = 515,
		KNIFE_SHADOW_DAGGERS = 516,
	};


public:
	KnifeData   m_knife_data[knives_t::KNIFE_MAX];
	GloveData   m_glove_data[gloves_t::GLOVE_MAX];
	int         m_last_seq;
	CBaseHandle m_glove_handle;
	bool        m_update;
	float       m_update_time;

public:
	void load();
	void think();
	void UpdateItem(C_WeaponCSBaseGun* item);
	void UpdateAnimations(C_CSPlayer* ent);
};

extern Skins g_skins;
