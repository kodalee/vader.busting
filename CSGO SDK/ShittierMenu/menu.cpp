#include "Menu.hpp"
#include "Fonts.h"
#include <chrono>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "IMGAY/imgui_internal.h"
#include "IMGAY/impl/imgui_impl_dx9.h"
#include "IMGAY/impl/imgui_impl_win32.h"

//lua
#include "../lua/clua.h"
#include "../lua/clua_hooks.h"
//


namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
		if (ImGui::CalcTextSize(label).x > 0) {
			ImGui::Text(label);
		}
		ImGui::SameLine();

        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(std::string("##" + std::string(label)).c_str(), &clr.x, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip)) {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
            return true;
        }
        return false;
    }
	inline bool ColorEdit4(const char* label, FloatColor* col, bool show_alpha = true)
	{
		if (ImGui::CalcTextSize(label).x > 0) {
			ImGui::Text(label);
		}
		ImGui::SameLine();
		float converted_col[4] = { -1495394904.f,-1495394904.f,-1495394904.f, -1495394904.f };
		converted_col[0] = col->r;
		converted_col[1] = col->g;
		converted_col[2] = col->b;
		converted_col[3] = col->a;
		if (ImGui::ColorEdit4(std::string("##" + std::string(label)).c_str(), converted_col, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip)) {
			col->r = converted_col[0];
			col->g = converted_col[1];
			col->b = converted_col[2];
			col->a = converted_col[3];
			return true;
		}

		return false;
	}

    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

 
int tab = 0, aimbotTab = 1, rageTab = 0, legitTab = 0;

enum WeaponGroup_t {
	WEAPONGROUP_PISTOL,
	WEAPONGROUP_HEAVYPISTOL,
	WEAPONGROUP_RIFLE,
	WEAPONGROUP_SNIPER,
	WEAPONGROUP_AUTOSNIPER,
	WEAPONGROUP_SUBMACHINE,
	WEAPONGROUP_HEAVY,
	WEAPONGROUP_SHOTGUN,
	WEAPONGROUP_MAX
};

bool testbox = false, testbox1 = false;
void Ragebot()
{
	//autosniper, sniper, pistol, heavy pistol, other

	ImGuiStyle* style = &ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();
	{
		static int current_weapon = 0, rage_current_group = 0, current_group = 0;

		if (rageTab == -1) {
			InsertCheckbox(EnableRagebot, XorStr("Aimbot"), &g_Vars.rage.enabled);
			ImGui::Hotkey("##Aimkey", &g_Vars.rage.key.key, &g_Vars.rage.key.cond, ImVec2{ 60,20 });

			InsertCheckbox(SilentAim, XorStr("Silent aim"), &g_Vars.rage.silent_aim);
			InsertCheckbox(Autoshoot, XorStr("Automatic fire"), &g_Vars.rage.auto_fire);
		}

		CVariables::RAGE* rbot = nullptr;
		switch (rageTab) {
		case WEAPONGROUP_PISTOL:
			rbot = &g_Vars.rage_pistols;
			break;
		case WEAPONGROUP_HEAVYPISTOL:
			rbot = &g_Vars.rage_heavypistols;
			break;
		case WEAPONGROUP_SUBMACHINE + 1:
			rbot = &g_Vars.rage_smgs;
			break;
		case WEAPONGROUP_RIFLE:
			rbot = &g_Vars.rage_rifles;
			break;
		case WEAPONGROUP_SHOTGUN + 1:
			rbot = &g_Vars.rage_shotguns;
			break;
		case WEAPONGROUP_SNIPER:
			rbot = &g_Vars.rage_awp;
			break;
		case WEAPONGROUP_SNIPER + 1:
			rbot = &g_Vars.rage_scout;
			break;
		case WEAPONGROUP_HEAVY + 1:
			rbot = &g_Vars.rage_heavys;
			break;
		case WEAPONGROUP_AUTOSNIPER + 1:
			rbot = &g_Vars.rage_autosnipers;
			break;
		case -1:
			rbot = &g_Vars.rage_default;
			break;
		default:
			rbot = &g_Vars.rage_default;
			break;
		}

		if (rageTab != -1) {
			InsertCheckbox(OverrideWeaponGroup, XorStr("Override weapon group") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->active);
		}
		const char* TargetSelection[] = { XorStr("Highest damage"), XorStr("Nearest to crosshair"), XorStr("Lowest health"), XorStr("Lowest distance"), XorStr("Lowest latency") };
		InsertCombo(std::string(XorStr("Target selection") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->target_selection, TargetSelection);

		std::vector<MultiItem_t> hitboxes = {
			{ XorStr("Head"), &rbot->hitboxes_head },
			{ XorStr("Neck"), &rbot->hitboxes_neck },
			{ XorStr("Chest"), &rbot->hitboxes_chest },
			{ XorStr("Stomach"), &rbot->hitboxes_stomach },
			{ XorStr("Pelvis"), &rbot->hitboxes_pelvis },
			{ XorStr("Arms"), &rbot->hitboxes_arms },
			{ XorStr("Legs"), &rbot->hitboxes_legs },
			{ XorStr("Feet"), &rbot->hitboxes_feets },
		};
		InsertMultiCombo(std::string(XorStr("Hitboxes") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), hitboxes);

		InsertCheckbox(IgnoreLimbs, XorStr("Ignore limbs when moving") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->ignorelimbs_ifwalking);
		InsertCheckbox(OverrideHitscan, XorStr("Override hitscan") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->override_hitscan);
		ImGui::Hotkey("##OverrideHitscanKey", &g_Vars.rage.override_key.key, &g_Vars.rage.override_key.cond, ImVec2{ 60,20 });
		
		if (rbot->override_hitscan) {
			std::vector<MultiItem_t> override_hitboxes = {
				{ XorStr("Head"), &rbot->bt_hitboxes_head },
				{ XorStr("Neck"), &rbot->bt_hitboxes_neck },
				{ XorStr("Chest"), &rbot->bt_hitboxes_chest },
				{ XorStr("Stomach"), &rbot->bt_hitboxes_stomach },
				{ XorStr("Pelvis"), &rbot->bt_hitboxes_pelvis },
				{ XorStr("Arms"), &rbot->bt_hitboxes_arms },
				{ XorStr("Legs"), &rbot->bt_hitboxes_legs },
				{ XorStr("Feet"), &rbot->bt_hitboxes_feets },
			};

			InsertMultiCombo(std::string(XorStr("Override hitboxes") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), override_hitboxes);
		}

		std::vector<MultiItem_t> mp_safety_hitboxes = {
			{ XorStr("Head"), &rbot->mp_hitboxes_head },
			{ XorStr("Chest"), &rbot->mp_hitboxes_chest },
			{ XorStr("Stomach"), &rbot->mp_hitboxes_stomach },
			{ XorStr("Legs"), &rbot->mp_hitboxes_legs },
			{ XorStr("Feet"), &rbot->mp_hitboxes_feets },
		};

		//	if (!mp_safety_hitboxes.empty())
		//		GUI::Controls::MultiDropdown(XorStr("Multipoints") + std::string(XorStr("#") + std::to_string(rage_current_group)), mp_safety_hitboxes);

		InsertCheckbox(StaticPointscale, XorStr("Static pointscale") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->static_point_scale);
		InsertSliderFloat(std::string(XorStr("Point scale##687687675") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->point_scale, 1.f, 100.0f, XorStr("%.0f%%"));
		InsertSliderFloat(std::string(XorStr("Stomach scale##68776678") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->body_point_scale, 1.f, 100.0f, XorStr("%.0f%%"));
		InsertSliderFloat(std::string(XorStr("Minimum hitchance") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->hitchance, 0.f, 100.f, XorStr("%.0f%%"));
		InsertSliderInt(std::string(XorStr("Minimum dmg") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->min_damage_visible, 1, 130, std::string(rbot->min_damage_visible > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage_visible - 100)))) : XorStr("%d hp")).c_str());
		InsertCheckbox(AutomaticPenetration, XorStr("Automatic penetration") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->autowall);
		if (rbot->autowall) {
			InsertSliderInt(std::string(XorStr("Minimum penetration dmg") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->min_damage, 1.f, 130.f, std::string(rbot->min_damage > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage - 100)))) : XorStr("%d hp")).c_str());
		}

		InsertCheckbox(Doubletap, XorStr("Doubletap") + std::string(XorStr("##") + std::to_string(rage_current_group)), &g_Vars.rage.exploit);
		ImGui::Hotkey("##DTkey", &g_Vars.rage.key_dt.key, &g_Vars.rage.key_dt.cond, ImVec2{ 60,20 });

		InsertCheckbox(MinDmgOverride, XorStr("Minimum dmg override") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->min_damage_override);
		ImGui::Hotkey("##MinDmgOverride", &g_Vars.rage.key_dmg_override.key, &g_Vars.rage.key_dmg_override.cond, ImVec2{ 60,20 });
		//ImGui::Keybind(std::string(XorStr("Minimum dmg override key#key") + std::string(XorStr("#") + std::to_string(rage_current_group))).c_str(), &g_Vars.rage.key_dmg_override.key);
		InsertSliderInt(std::string(XorStr("Dmg override amount##slider") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->min_damage_override_amount, 1, 130, std::string(rbot->min_damage_override_amount > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage_override_amount - 100)))) : XorStr("%d hp")).c_str());

		ImGui::NextColumn();
		ImGui::NewLine();

		InsertCheckbox(ExtendedBT, std::string(XorStr("Extended backtrack") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &g_Vars.misc.extended_backtrack);
		if (g_Vars.misc.extended_backtrack) {
			ImGui::Hotkey(std::string(XorStr("Extended backtrack qswdecxvrftgbyhnujmiko,l") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &g_Vars.misc.extended_backtrack_key.key, &g_Vars.misc.extended_backtrack_key.cond, ImVec2{ 60,20 });
			InsertSliderFloat(std::string(XorStr("Extended backtrack##amt") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &g_Vars.misc.extended_backtrack_time, 0.f, 1.f, XorStr("%.2fs"));
		}

		InsertCheckbox(CompensateSpreadRage, std::string(XorStr("Compensate spread") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->compensate_spread);

		InsertCheckbox(Autostop, XorStr("Automatic stop"), &rbot->autostop_check);
		std::vector<MultiItem_t> stop_options = {
			{ XorStr("Always stop"), &rbot->always_stop },
			{ XorStr("Between shots"), &rbot->between_shots },
			{ XorStr("Early"), &rbot->early_stop },

		};

		InsertMultiCombo(XorStr("Automatic stop options"), stop_options);

		InsertCheckbox(Autoscope, XorStr("Automatic scope") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->autoscope);
		InsertCheckbox(prefer_body, XorStr("Prefer body-aim") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->prefer_body);

		if (rbot->prefer_body) {
			std::vector<MultiItem_t> prefer_body_cond = {
				//	{ XorStr( "Target firing" ), &rbot->prefer_body_disable_shot },
				{ XorStr("Target resolved"), &rbot->prefer_body_disable_resolved },
				//	{ XorStr( "Safe point headshot" ), &rbot->prefer_body_disable_safepoint_head }
			};

			GUI::Controls::MultiDropdown(XorStr("Prefer body-aim disablers##PreferBody") + std::string(XorStr("##") + std::to_string(rage_current_group)), prefer_body_cond);
		}

		InsertCheckbox(AccuracyBoost, XorStr("Accuracy boost") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->accry_boost_on_shot);

		const char* accuracyonshotmodes[] = {XorStr("Off"), XorStr("Low"), XorStr("Medium"), XorStr("High")};
		InsertCombo(XorStr("Accuracy boost modes"), &rbot->accry_boost_on_shot_modes, accuracyonshotmodes);

		InsertCheckbox(shotdelay, XorStr("Delay hitbox selection") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->shotdelay);
		InsertCheckbox(delay_shot_on_unducking, XorStr("Delay shot on unduck") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->delay_shot_on_unducking);

		InsertCheckbox(knife_bot, XorStr("Knife bot"), &g_Vars.misc.knife_bot);
		const char* knife_bot_type[] = { XorStr("Default"), XorStr("Backstab"), XorStr("Quick") };
		if (g_Vars.misc.knife_bot)
			InsertCombo(XorStr("Knifebot type##Knife bot type"), &g_Vars.misc.knife_bot_type, knife_bot_type);

		InsertCheckbox(zeus_bot, XorStr("Zeus bot"), &g_Vars.misc.zeus_bot);
		if (g_Vars.misc.zeus_bot)
			InsertSliderFloat(XorStr("Zeus bot hitchance"), &g_Vars.misc.zeus_bot_hitchance, 1.f, 80.f, XorStr("%.0f%%"));

		ImGui::Text(std::string(XorStr("Force bodyaim") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str());
		ImGui::Hotkey(std::string(XorStr("Force bodyaim key##key") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &g_Vars.rage.prefer_body.key, &g_Vars.rage.prefer_body.cond, ImVec2{ 60,20 });

		ImGui::Text(std::string(XorStr("Override resolver") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str());
		ImGui::Hotkey(std::string(XorStr("Override resolver key##key") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &g_Vars.rage.override_reoslver.key, &g_Vars.rage.override_reoslver.cond, ImVec2{ 60,20 });
	}
	ImGui::EndColumns();
}

void HvH()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	const char* styles[]{ "Static","Jitter","Spin" };

	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();
	{
		CVariables::ANTIAIM_STATE* settings = &g_Vars.antiaim_stand;
		// enable AA.
		InsertCheckbox(AntiAim, "Anti-Aim", &g_Vars.antiaim.enabled)
		if (g_Vars.antiaim.enabled) {
			// yaw / pitch / fake.

			const char* pitches[] = { XorStr("Off"), XorStr("Down"), XorStr("Up"), XorStr("Zero") };
			InsertCombo(XorStr("Pitch"), &settings->pitch, pitches);

			const char* real_yaw[] = { XorStr("Off"), XorStr("180"), XorStr("Freestand"), XorStr("180z") };
			InsertCombo(XorStr("Real yaw"), &settings->base_yaw, real_yaw);


			const char* fake_yaw[] = { XorStr("Off"), XorStr("Dynamic"), XorStr("Sway"), XorStr("Static") };
			InsertCombo(XorStr("Real yaw"), &settings->yaw, fake_yaw);

			// static lets choose our own vaule.
			if (settings->yaw == 3) {
				InsertSliderInt(XorStr("Break angle"), &g_Vars.antiaim.break_lby, -145, 145, "%d");
			}

			if (settings->base_yaw == 0) {}

			//InsertCheckbox( XorStr( "Freestand yaw" ), &g_Vars.antiaim.freestand );
			InsertCheckbox(AntiAimPreserve, XorStr("Preserve stand yaw"), &g_Vars.antiaim.preserve);
			InsertCheckbox(AntiAimManual, XorStr("Manual"), &g_Vars.antiaim.manual); ImGui::SameLine(); ImGuiEx::ColorEdit4(XorStr("Manual color"), &g_Vars.antiaim.manual_color);
			if (g_Vars.antiaim.manual) {
				
				ImGui::Text(XorStr("Left"));
				ImGui::Hotkey(XorStr("Left key##key"), &g_Vars.antiaim.manual_left_bind.key, &g_Vars.antiaim.manual_left_bind.cond, ImVec2{ 60,20 });

				ImGui::Text(XorStr("Right"));
				ImGui::Hotkey(XorStr("Right key##key"), &g_Vars.antiaim.manual_right_bind.key, &g_Vars.antiaim.manual_right_bind.cond, ImVec2{ 60,20 });

				ImGui::Text(XorStr("Back"));
				ImGui::Hotkey(XorStr("Back key##key"), &g_Vars.antiaim.manual_back_bind.key, &g_Vars.antiaim.manual_back_bind.cond, ImVec2{ 60,20 });
			}

			// distortion.
			InsertCheckbox(Distortion, XorStr("Distortion###sse"), &g_Vars.antiaim.distort)
			if (g_Vars.antiaim.distort) {
				// manual aa is on, show this.
				InsertCheckbox(AAManualOverride, XorStr("Manual override"), &g_Vars.antiaim.distort_manual_aa);
				InsertCheckbox(Twist, XorStr("Twist"), &g_Vars.antiaim.distort_twist);
				if (g_Vars.antiaim.distort_twist)
					InsertSliderFloat(XorStr("Speed"), &g_Vars.antiaim.distort_speed, 1.f, 10.f, XorStr("%.1fs"));
				InsertSliderFloat(XorStr("Max time"), &g_Vars.antiaim.distort_max_time, 0.f, 10.f, "%f");
				InsertSliderFloat(XorStr("Range"), &g_Vars.antiaim.distort_range, -360.f, 360.f, "%f");

				std::vector<MultiItem_t> distort_disablers = {
					{ XorStr("Fakewalking"), &g_Vars.antiaim.distort_disable_fakewalk },
				{ XorStr("Running"), &g_Vars.antiaim.distort_disable_run },
				{ XorStr("Airborne"), &g_Vars.antiaim.distort_disable_air },
				};

				InsertMultiCombo(XorStr("Distortion disablers"), distort_disablers);
			}
		}
		//GUI::Controls::Label( XorStr( "Invert LBY" ) );
		//GUI::Controls::Hotkey( XorStr( "LBY Flip" ), &g_Vars.antiaim.desync_flip_bind );
		//InsertCheckbox( XorStr( "Imposter breaker" ), &g_Vars.antiaim.imposta );

		InsertCheckbox(FakeWalk, XorStr("Fake-walk"), &g_Vars.misc.slow_walk);
		ImGui::Hotkey(XorStr("Fake-walk key#Key"), &g_Vars.misc.slow_walk_bind.key, &g_Vars.misc.slow_walk_bind.cond, ImVec2{ 60,20 });
		InsertSliderInt(XorStr("Fake-walk speed"), &g_Vars.misc.slow_walk_speed, 4, 16, "%d");

		ImGui::NextColumn();
		ImGui::NewLine();

		InsertCheckbox(Fakelag, XorStr("Fakelag##fakeKURWAlag"), &g_Vars.fakelag.enabled);

		std::vector<MultiItem_t> fakelag_cond = { { XorStr("Moving"), &g_Vars.fakelag.when_moving }, { XorStr("In air"), &g_Vars.fakelag.when_air }, };
		InsertMultiCombo(XorStr("Conditions"), fakelag_cond);

		const char* FakelagType[] = { XorStr("Maximum"), XorStr("Dynamic"), XorStr("Fluctuate") };
		InsertCombo(XorStr("Type"), &g_Vars.fakelag.choke_type, FakelagType);
		InsertSliderInt(XorStr("Limit"), &g_Vars.fakelag.choke, 0, 16, "%d");
		InsertSliderInt(XorStr("Double-tap Limit"), &g_Vars.fakelag.dt_choke, 0, 16, "%d");

		g_Vars.fakelag.trigger_duck = g_Vars.fakelag.trigger_weapon_activity = g_Vars.fakelag.trigger_shooting = false;
		g_Vars.fakelag.trigger_land = true;
		g_Vars.fakelag.alternative_choke = 0;

		InsertSliderFloat(XorStr("Variance"), &g_Vars.fakelag.variance, 0.0f, 100.0f, XorStr("%.0f %%"));
		
	}
	ImGui::EndColumns();
}

void Visuals()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);

	ImGui::NewLine();
	{
		InsertCheckbox(esp_enable, XorStr("ESP"), &g_Vars.esp.esp_enable);
		g_Vars.esp.team_check = true;

		InsertCheckbox(fade_esp, XorStr("Dormant"), &g_Vars.esp.fade_esp);
		//ImGuiEx::ColorEdit4( XorStr( "dormant clr" ), &g_Vars.esp.dormant_color );

		InsertCheckbox(box, XorStr("Box"), &g_Vars.esp.box);
		ImGuiEx::ColorEdit4(XorStr("Box color"), &g_Vars.esp.box_color);

		InsertCheckbox(offscren_enabled, XorStr("Offscreen"), &g_Vars.esp.offscren_enabled);
		if (g_Vars.esp.offscren_enabled) {
			ImGuiEx::ColorEdit4(XorStr("Offscreen color"), &g_Vars.esp.offscreen_color);
			GUI::Controls::Slider(XorStr("Offscreen distance"), &g_Vars.esp.offscren_distance, 10, 100.f, XorStr("%.0f%%"));
			GUI::Controls::Slider(XorStr("Offscreen size"), &g_Vars.esp.offscren_size, 4, 16.f, XorStr("%.0fpx"));
		}

		InsertCheckbox(name, XorStr("Name"), &g_Vars.esp.name);
		ImGuiEx::ColorEdit4(XorStr("Name color"), &g_Vars.esp.name_color);

		InsertCheckbox(health, XorStr("Health"), &g_Vars.esp.health);
		InsertCheckbox(health_override, XorStr("Health color override"), &g_Vars.esp.health_override);
		ImGuiEx::ColorEdit4(XorStr("Health color"), &g_Vars.esp.health_color);

		InsertCheckbox(skeleton, XorStr("Skeleton"), &g_Vars.esp.skeleton);
		ImGuiEx::ColorEdit4(XorStr("Skeleton color"), &g_Vars.esp.skeleton_color);

		InsertCheckbox(ammo, XorStr("Ammo"), &g_Vars.esp.draw_ammo_bar);
		ImGuiEx::ColorEdit4(XorStr("Ammo color"), &g_Vars.esp.ammo_color);

		InsertCheckbox(lby_timer, XorStr("LBY timer"), &g_Vars.esp.draw_lby_bar);
		ImGuiEx::ColorEdit4("LBY color", &g_Vars.esp.lby_color);

		InsertCheckbox(weapon, XorStr("Weapon"), &g_Vars.esp.weapon);
		ImGuiEx::ColorEdit4(XorStr("Weapon color"), &g_Vars.esp.weapon_color);

		InsertCheckbox(weapon_icon, XorStr("Weapon icon"), &g_Vars.esp.weapon_icon);
		ImGuiEx::ColorEdit4(XorStr("Weapon icon color"), &g_Vars.esp.weapon_icon_color);

		ImGui::NextColumn();
		ImGui::NewLine();

		InsertCheckbox(enemy_chams, XorStr("Enemy chams"), &g_Vars.esp.chams_enemy);
		if (g_Vars.esp.chams_enemy) {
			InsertCheckbox(enemy_chams_vis, XorStr("Visible chams"), &g_Vars.esp.enemy_chams_vis);
			ImGuiEx::ColorEdit4(XorStr("Visible chams color##Local color"), &g_Vars.esp.enemy_chams_color_vis);

			InsertCheckbox(enemy_chams_xqz, XorStr("Behind wall chams"), &g_Vars.esp.enemy_chams_xqz);
			ImGuiEx::ColorEdit4(XorStr("Behind wall chams color##Local color"), &g_Vars.esp.enemy_chams_color_xqz);

			const char* enemy_chams_mat[] = {
				XorStr("Off"), XorStr("Flat"), XorStr("Material"), XorStr("Ghost"), XorStr("Outline"), XorStr("Shiny")
			};
			InsertCombo(XorStr("Enemy chams material"), &g_Vars.esp.enemy_chams_mat, enemy_chams_mat);

			if (g_Vars.esp.enemy_chams_mat == 5) {
				GUI::Controls::Slider(XorStr("Enemy pearlescence"), &g_Vars.esp.chams_enemy_pearlescence, 0.f, 100.f, XorStr("%0.0f"));
				ImGuiEx::ColorEdit4(XorStr("clacoenemycascans"), &g_Vars.esp.chams_enemy_pearlescence_color);
				GUI::Controls::Slider(XorStr("Enemy shine"), &g_Vars.esp.chams_enemy_shine, 0.f, 100.f, XorStr("%0.0f"));
			}

			InsertCheckbox(glow_enemy, XorStr("Glow"), &g_Vars.esp.glow_enemy);
			ImGuiEx::ColorEdit4(XorStr("Glow color#Enemy glow"), &g_Vars.esp.glow_enemy_color);

			InsertCheckbox(chams_enemy_outline, XorStr("Add glow#Enemy"), &g_Vars.esp.chams_enemy_outline); ImGuiEx::ColorEdit4(XorStr("Add glow color#Enemy"), &g_Vars.esp.chams_enemy_outline_color);
			if (g_Vars.esp.chams_enemy_outline) {
				InsertCheckbox(chams_enemy_outline_wireframe, XorStr("Wireframe#Enemy"), &g_Vars.esp.chams_enemy_outline_wireframe);
				InsertSliderFloat(XorStr("Glow strength#Enemy"), &g_Vars.esp.chams_enemy_outline_value, 1.f, 100.f, XorStr("%0.0f"));
			}
		}


		const char* materials[] = {
XorStr("Off"), XorStr("Flat"), XorStr("Material"), XorStr("Ghost"), XorStr("Outline")
		};

		InsertCheckbox(chams_history, XorStr("Backtrack chams"), &g_Vars.esp.chams_history);
		if (g_Vars.esp.chams_history) {
			ImGuiEx::ColorEdit4(XorStr("Backtrack chams color##Local color"), &g_Vars.esp.chams_history_color);

			InsertCombo(XorStr("Backtrack chams material"), &g_Vars.esp.chams_history_mat, materials);
		}

		InsertCheckbox(hitmatrix, XorStr("Shot chams"), &g_Vars.esp.hitmatrix);
		if (g_Vars.esp.hitmatrix) {
			ImGuiEx::ColorEdit4(XorStr("Shot chams color##Local color"), &g_Vars.esp.hitmatrix_color);
			InsertSliderFloat(XorStr("Expire time##chams"), &g_Vars.esp.hitmatrix_time, 1.f, 10.f, XorStr("%0.0f seconds"));
		}

		InsertCheckbox(glow_local, XorStr("Local glow"), &g_Vars.esp.glow_local);
		if (g_Vars.esp.glow_local) {
			ImGuiEx::ColorEdit4(XorStr("Local glow color##Local color"), &g_Vars.esp.glow_local_color);
		}

		InsertCheckbox(chams_local, XorStr("Local chams"), &g_Vars.esp.chams_local);
		if (g_Vars.esp.chams_local) {
			ImGuiEx::ColorEdit4(XorStr("Local chams color##Local color"), &g_Vars.esp.chams_local_color);


			const char* chams_local_mat[] = {
				XorStr("Off"), XorStr("Flat"), XorStr("Material"), XorStr("Ghost"), XorStr("Outline"), XorStr("Shiny")
			};
			InsertCombo(XorStr("Local chams material"), &g_Vars.esp.chams_local_mat, chams_local_mat);

			if (g_Vars.esp.chams_local_mat == 5) {
				GUI::Controls::Slider(XorStr("Local pearlescence"), &g_Vars.esp.chams_local_pearlescence, 0.f, 100.f, XorStr("%0.0f"));
				ImGuiEx::ColorEdit4(XorStr("clacolacscascans"), &g_Vars.esp.chams_local_pearlescence_color);
				GUI::Controls::Slider(XorStr("Local shine"), &g_Vars.esp.chams_local_shine, 0.f, 100.f, XorStr("%0.0f"));
			}

			InsertCheckbox(chams_local_outline, XorStr("Add glow local#Local"), &g_Vars.esp.chams_local_outline);
			if (g_Vars.esp.chams_local_outline) {
				ImGuiEx::ColorEdit4(XorStr("Add glow local color#Local"), &g_Vars.esp.chams_local_outline_color);
				InsertCheckbox(chams_local_outline_wireframe, XorStr("Wireframe local#Local"), &g_Vars.esp.chams_local_outline_wireframe);

				GUI::Controls::Slider(XorStr("Local glow strength#Local"), &g_Vars.esp.chams_local_outline_value, 1.f, 100.f, XorStr("%0.0f"));
			}
		}

		InsertCheckbox(blur_in_scoped, XorStr("Transparency in scope"), &g_Vars.esp.blur_in_scoped);
		InsertSliderFloat(XorStr("Transparency##Transparency In Scope"), &g_Vars.esp.blur_in_scoped_value, 0.0f, 100.f, XorStr("%0.f%%"));

		InsertCheckbox(chams_attachments, XorStr("Attachment chams"), &g_Vars.esp.chams_attachments);
		if (g_Vars.esp.chams_attachments) {
			ImGuiEx::ColorEdit4(XorStr("Attachment chams color"), &g_Vars.esp.attachments_chams_color);
			InsertCombo(XorStr("Attachment chams material"), &g_Vars.esp.attachments_chams_mat, materials);

			InsertCheckbox(chams_attachments_outline, XorStr("Add glow attachment#Attachment"), &g_Vars.esp.chams_attachments_outline);
			if (g_Vars.esp.chams_attachments_outline) {
				ImGuiEx::ColorEdit4(XorStr("Add glow attachment color#Attachment"), &g_Vars.esp.chams_attachments_outline_color);
				InsertCheckbox(chams_attachments_outline_wireframe, XorStr("Wireframe attachment#Attachment"), &g_Vars.esp.chams_attachments_outline_wireframe);

				InsertSliderFloat(XorStr("Attachment glow strength#Attachment"), &g_Vars.esp.chams_attachments_outline_value, 1.f, 100.f, XorStr("%0.0f"));
			}
		}

		InsertCheckbox(chams_weapon, XorStr("Weapon chams"), &g_Vars.esp.chams_weapon);
		if (g_Vars.esp.chams_weapon) {
			ImGuiEx::ColorEdit4(XorStr("Weapon chams color"), &g_Vars.esp.weapon_chams_color);
			InsertCombo(XorStr("Weapon chams material"), &g_Vars.esp.weapon_chams_mat, materials);

			InsertCheckbox(chams_weapon_outline, XorStr("Add glow weapon#Weapon"), &g_Vars.esp.chams_weapon_outline);
			if (g_Vars.esp.chams_weapon_outline) {
				ImGuiEx::ColorEdit4(XorStr("Add glow weapon color#Weapon"), &g_Vars.esp.chams_weapon_outline_color);
				InsertCheckbox(chams_weapon_outline_wireframe, XorStr("Wireframe weapon#Weapon"), &g_Vars.esp.chams_weapon_outline_wireframe);

				GUI::Controls::Slider(XorStr("Weapon glow strength#Weapon"), &g_Vars.esp.chams_weapon_outline_value, 1.f, 100.f, XorStr("%0.0f"));
			}
		}

		InsertCheckbox(chams_hands, XorStr("Hand chams"), &g_Vars.esp.chams_hands);
		if (g_Vars.esp.chams_hands) {
			ImGuiEx::ColorEdit4(XorStr("Hand chams color"), &g_Vars.esp.hands_chams_color);
			InsertCombo(XorStr("Hand chams material"), &g_Vars.esp.hands_chams_mat, materials);


			InsertCheckbox(chams_hands_outline, XorStr("Add glow hand#Hand"), &g_Vars.esp.chams_hands_outline);
			if (g_Vars.esp.chams_hands_outline) {
				ImGuiEx::ColorEdit4(XorStr("Add glow hand color#Hand"), &g_Vars.esp.chams_hands_outline_color);
				InsertCheckbox(chams_hands_outline_wireframe, XorStr("Wireframe hand#Hand"), &g_Vars.esp.chams_hands_outline_wireframe);

				InsertSliderFloat(XorStr("Hand glow strength#Hand"), &g_Vars.esp.chams_hands_outline_value, 1.f, 100.f, XorStr("%0.0f"));
			}
		}

		InsertCheckbox(draw_hitboxes, XorStr("Shot capsules"), &g_Vars.esp.draw_hitboxes);
		if (g_Vars.esp.draw_hitboxes) {
			ImGuiEx::ColorEdit4(XorStr("Shot capsules color"), &g_Vars.esp.hitboxes_color);
		}

	}
	ImGui::EndColumns();
}

void Misc()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);

	
	ImGui::NewLine();
	{

	}
	ImGui::EndColumns();
}

void Scripts()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);


	ImGui::NewLine();
	{

		ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(75.f); ImGui::PushItemWidth(158.f);  if (ImGui::Button("Refresh scripts", ImVec2(100, 0))) g_lua.refresh_scripts(); ImGui::PopItemWidth(); ImGui::CustomSpacing(1.f);

		ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(75.f); ImGui::PushItemWidth(158.f);  if (ImGui::Button("Reload active scripts", ImVec2(100, 0))) g_lua.reload_all_scripts(); ImGui::PopItemWidth(); ImGui::CustomSpacing(1.f);

		ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(75.f); ImGui::PushItemWidth(158.f);  if (ImGui::Button("Unload all", ImVec2(100, 0))) g_lua.unload_all_scripts(); ImGui::PopItemWidth(); ImGui::CustomSpacing(1.f);

		ImGui::Text("scripts");

		for (auto s : g_lua.scripts)
		{
			ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f);
			if (ImGui::Selectable(s.c_str(), g_lua.loaded.at(g_lua.get_script_id(s)), NULL, ImVec2(0, 0))) {
				auto scriptId = g_lua.get_script_id(s);
				if (g_lua.loaded.at(scriptId))
					g_lua.unload_script(scriptId);
				else
					g_lua.load_script(scriptId);
			}
		}

	}
	ImGui::EndColumns();
}

bool IMGUIMenu::Initialize(IDirect3DDevice9* pDevice)
{
	static bool initialized = false;
	if (!initialized)
	{
		ImGui::CreateContext();
		CreateStyle();
		IDirect3DSwapChain9* pChain = nullptr;
		D3DPRESENT_PARAMETERS pp = {};
		D3DDEVICE_CREATION_PARAMETERS param = {};
		pDevice->GetCreationParameters(&param);
		pDevice->GetSwapChain(0, &pChain);

		if (pChain) pChain->GetPresentParameters(&pp);

		ImGui_ImplWin32_Init(param.hFocusWindow);
		ImGui_ImplDX9_Init(pDevice);
		ImGui_ImplDX9_CreateDeviceObjects();
		initialized = true;
	}

	return initialized;
}

void IMGUIMenu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void IMGUIMenu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void IMGUIMenu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

auto windowFlags = (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding);
ImFont* gravity, *gravityBold, *StarWars, *watermark;

void IMGUIMenu::Render()
{
	if (!Opened) return;

	//ImGui::GetIO().MouseDrawCursor = _visible;

	ImGuiStyle* style = &ImGui::GetStyle();

	ImGui::PushFont(gravityBold);

	ImGui::SetNextWindowSize(ImVec2(600.f, 550.f));
	ImGui::Begin("##menu", &_visible, windowFlags);

	style->WindowPadding = ImVec2(7.f, 7.f);

	style->Colors[ImGuiCol_MenuAccent] = ImColor(255, 215, 0);
	style->Colors[ImGuiCol_Logo] = ImColor(0, 87, 255);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	{
		ImGui::BeginTitleBar("Title Bar", ImVec2(586.f, 55.f), false);

		ImGui::PopFont();
		ImGui::PushFont(StarWars);
		ImGui::SameLine(5.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImGuiCol_Logo);
		ImGui::Text("VADER");
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::PushFont(gravityBold);

		ImGui::EndTitleBar();
	}
	ImGui::PopStyleVar();
	style->Colors[ImGuiCol_ChildBg] = ImColor(41, 32, 59);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	{
		ImGui::BeginChild("##tabs", ImVec2(586.f, 35.f), false);

		ImGui::SameLine(8.f);
		ImGui::TrueTab("  AIMBOT  ", tab, 0, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  HVH  ", tab, 1, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  VISUALS  ", tab, 2, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  MISC  ", tab, 3, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  SKINS  ", tab, 4, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  SCRIPTS  ", tab, 5, ImVec2(0.f, 35.f)); ImGui::SameLine();

		ImGui::EndChild();
	}
	ImGui::PopStyleVar();

	style->Colors[ImGuiCol_ChildBg] = ImColor(31, 24, 46);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); /* all SubTabs are handled here */
	{
		ImGui::BeginChild("##subtabs", ImVec2(586.f, 25.f), false);

		ImGui::SameLine(8.f);


		switch (tab)
		{
			case 0:
			{
				ImGui::TrueSubTab("  Default  ", rageTab, -1, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Pistols  ", rageTab, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Heavy pistols  ", rageTab, 1, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Rifles  ", rageTab, 2, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  AWP  ", rageTab, 3, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Scout  ", rageTab, 4, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Auto  ", rageTab, 5, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  SMG  ", rageTab, 6, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Heavys  ", rageTab, 7, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Shotguns  ", rageTab, 8, ImVec2(0.f, 25.f)); ImGui::SameLine();

				break;
			}
			case 1:
			{
				ImGui::SelectedSubTab("  Basic  ", ImVec2(0.f, 25.f));
				break;
			}
			case 2:
			{
				ImGui::SelectedSubTab("  Player  ", ImVec2(0.f, 25.f));
				break;
			}
			case 3:
			{
				ImGui::SelectedSubTab("  Misc  ", ImVec2(0.f, 25.f));
				break;
			}
		}

		ImGui::EndChild();
	}
	ImGui::PopStyleVar();

	style->Colors[ImGuiCol_ChildBg] = ImColor(25, 20, 27);
	ImGui::BeginChild("##main", ImVec2(586.f, 420.f), false);
	ImGui::SameLine(7.f);
	style->Colors[ImGuiCol_ChildBg] = ImColor(21, 17, 29);
	switch (tab)
	{
		case 0:
			Ragebot();
			break;
		case 1:
			HvH();
			break;
		case 2:
			Visuals();
			break;
		case 3:
			Misc();
		case 5:
			Scripts();
		default:
			break;
	}
	style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
	ImGui::EndChild();
	ImGui::End();
	ImGui::PopFont();
  
}

void IMGUIMenu::Toggle()
{
    _visible = !_visible;
}

void IMGUIMenu::CreateStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::StyleColorsDark(&style);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.GrabMinSize = 5.0f;
	style.ScrollbarSize = 12.f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.FrameBorderSize = 0.4f;
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	//io.MouseDrawCursor = true;
	char buffer[MAX_PATH];
	//GetWindowsDirectoryA(buffer, MAX_PATH);

	gravity = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravity_compressed_data, gravity_compressed_size, 13.f);
	StarWars = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(star_wars_compressed_data, star_wars_compressed_size, 50.f);
	watermark = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 16.f);
	gravityBold = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 13.f);
	ImGui::SmallestPixel = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(smallestpixel_compressed_data, smallestpixel_compressed_size, 10, NULL, io.Fonts->GetGlyphRangesCyrillic());

}

void CustomCheckbox::Draw(std::string title, bool* value)
{
	using namespace std::chrono;
	auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 10;

	auto balls = *value;

	if (!FirstDraw)
	{
		AnimStep = balls ? 200.f : 0.f;
		LastStep = ms;
		FirstDraw = true;
	}

	ImGui::CheckBoxAnimated((title).c_str(), value, AnimStep);


	auto delta = ms - LastStep;
	auto mod = 5 * delta; //9 / (1000 / 60) = 0.54
	AnimStep += balls ? mod : -mod;
	LastStep = ms;

	if (AnimStep <= 0) {
		AnimStep = 0;
		LastStep = ms;
	}
	else if (AnimStep >= 200) {
		AnimStep = 200;
		LastStep = ms;
	}
}