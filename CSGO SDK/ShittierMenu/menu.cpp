#include "Menu.hpp"
#include "Fonts.h"
#include <chrono>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "IMGAY/imgui_internal.h"
#include "IMGAY/impl/imgui_impl_dx9.h"
#include "IMGAY/impl/imgui_impl_win32.h"


namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip)) {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
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
void Legitbot()
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

		InsertCheckbox(EnableRagebot, XorStr("Enable"), &g_Vars.rage.enabled); ImGui::SameLine();
		ImGui::Hotkey("##Aimkey", &g_Vars.rage.key.key, &g_Vars.rage.key.cond, ImVec2{ 60,20 });

		InsertCheckbox(SilentAim, XorStr("Silent aim"), &g_Vars.rage.silent_aim);
		InsertCheckbox(Autoshoot, XorStr("Automatic fire"), &g_Vars.rage.auto_fire);

		CVariables::RAGE* rbot = nullptr;
		switch (rageTab - 1) {
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
		default:
			rbot = &g_Vars.rage_default;
			break;
		}

		InsertCheckbox(OverrideWeaponGroup, XorStr("Override weapon group") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->active);

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
		ImGui::SameLine();
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

			InsertMultiCombo(std::string(XorStr("Override hitboxes") + std::string(XorStr("#") + std::to_string(rage_current_group))).c_str(), override_hitboxes);
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
		ImGui::SameLine();
		ImGui::Hotkey("##DTkey", &g_Vars.rage.key_dt.key, &g_Vars.rage.key_dt.cond, ImVec2{ 60,20 });

		InsertCheckbox(MinDmgOverride, XorStr("Minimum dmg override") + std::string(XorStr("##") + std::to_string(rage_current_group)), &rbot->min_damage_override);
		ImGui::SameLine();
		ImGui::Hotkey("##MinDmgOverride", &g_Vars.rage.key_dmg_override.key, &g_Vars.rage.key_dmg_override.cond, ImVec2{ 60,20 });
		//ImGui::Keybind(std::string(XorStr("Minimum dmg override key#key") + std::string(XorStr("#") + std::to_string(rage_current_group))).c_str(), &g_Vars.rage.key_dmg_override.key);
		InsertSliderInt(std::string(XorStr("Dmg override amount##slider") + std::string(XorStr("##") + std::to_string(rage_current_group))).c_str(), &rbot->min_damage_override_amount, 1, 130, std::string(rbot->min_damage_override_amount > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage_override_amount - 100)))) : XorStr("%d hp")).c_str());
	}
	ImGui::EndColumns();
}

void Antiaim()
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
		ImGui::TrueTab("  ANTI-AIM  ", tab, 1, ImVec2(0.f, 35.f)); ImGui::SameLine();
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
				ImGui::TrueSubTab("  Pistols  ", rageTab, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Heavy pistols  ", rageTab, 1, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Rifles  ", rageTab, 2, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  AWP  ", rageTab, 3, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Scout  ", rageTab, 4, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Auto snipers  ", rageTab, 5, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Sub machines  ", rageTab, 6, ImVec2(0.f, 25.f)); ImGui::SameLine();
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
			(aimbotTab == 1) ? Ragebot() : Legitbot();
			break;
		case 1:
			Antiaim();
			break;
		case 2:
			Visuals();
			break;
		case 3:
			Misc();
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