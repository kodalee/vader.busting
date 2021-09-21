#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include <d3d9.h>
#include <d3dx9.h>

#include "Fonts.h"
#include <chrono>
#include "retard.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "IMGAY/imgui_internal.h"
#include "IMGAY/impl/imgui_impl_dx9.h"
#include "IMGAY/impl/imgui_impl_win32.h"

#include "../SDK/sdk.hpp"
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
        //    v->SetColor(clr.x, clr.y, clr.z, clr.w); ???????????
            return true;
        }
        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

#define InsertSliderInt(x1,x2,x3,x4,x5) ImGui::NewLine(); ImGui::PushItemWidth(159.f); ImGui::SliderInt(x1, x2, x3, x4, x5); ImGui::PopItemWidth();
#define InsertSliderFloat(x1,x2,x3,x4,x5) ImGui::NewLine(); ImGui::PushItemWidth(159.f); ImGui::SliderFloat(x1, x2, x3, x4, x5); ImGui::PopItemWidth();
#define InsertCombo(x1,x2,x3,x4) ImGui::NewLine(); ImGui::PushItemWidth(159.f); style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0); ImGui::Combo(x1, x2, x3, x4); style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0); ImGui::PopItemWidth();
#define InsertCheckbox(x1, x2, x3) static Checkbox x1; x1.Draw(x2,x3);

 
static int tab = 0;
static int aimbotTab = 1;
static int rageTab = 0;
static int legitTab = 0;

static Checkbox emptyc;

bool PlaceHolderEmptyFUckingPieceOFsSHIT = true;
#define InsertEmpty() emptyc.Draw("##", &PlaceHolderEmptyFUckingPieceOFsSHIT);

bool testbox = false, testbox1 = false;
void Legitbot()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	//ImGui::BeginGroupBox("Legitbot", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();


}
void Ragebot()
{
	//autosniper, sniper, pistol, heavy pistol, other

	ImGuiStyle* style = &ImGui::GetStyle();
	//ImGui::BeginGroupBox("Ragebot", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();

	InsertCheckbox(Ragebot, "Enable", &g_Vars.rage.enabled); ImGui::SameLine; ImGui::Hotkey("##enablekey", &g_Vars.rage.key.key, ImVec2{ 60,20 }); InsertEmpty();
	InsertCheckbox(Testbox, "Cum", &testbox1);


}

void Antiaim()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	const char* styles[]{ "Static","Jitter","Spin" };

	//ImGui::BeginGroupBox("Antiaim", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);

	ImGui::NewLine();

	//ImGui::Text("Inverter Key");
	//ImGui::Hotkey("##Invert", g_Options.inverter_key, ImVec2{ 159,20 });

		ImGui::EndColumns();
	//	ImGui::EndGroupBox();
}

void Visuals()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	//ImGui::BeginGroupBox("Visuals", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);

	ImGui::NewLine();

	ImGui::EndColumns();
	//ImGui::EndGroupBox();
}

void Misc()
{
	ImGuiStyle* style = &ImGui::GetStyle();

//	ImGui::BeginGroupBox("Misc", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	InsertEmpty();
		ImGui::NewLine();
		ImGui::Text("Misc");

	ImGui::EndColumns();
//	ImGui::EndGroupBox();
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

		if (pChain)
			pChain->GetPresentParameters(&pp);


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

auto windowFlags = (ImGuiWindowFlags_NoCollapse
    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding);

ImFont* gravity;
ImFont* gravityBold;
ImFont* Streamster;
ImFont* watermark;


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
			ImGui::PushFont(Streamster);
			ImGui::SameLine(5.f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGuiCol_Logo);
			//ImGui::Text("");
			ImGui::PopStyleColor();
			ImGui::PopFont();
			ImGui::PushFont(gravityBold);
		//	ImGui::Image(obam::globals::menuLogo, ImVec2(600.f, 55.f));
			//ImGui::SameLine(400.f);
			//ImGui::Text(loader_info.username);

			ImGui::EndTitleBar();
		}
		ImGui::PopStyleVar();
		style->Colors[ImGuiCol_ChildBg] = ImColor(41, 32, 59);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		{
			ImGui::BeginChild("Tabs", ImVec2(586.f, 35.f), false);

			ImGui::SameLine(8.f);

			switch (tab) {

			case 0:
				if (ImGui::SelectedTab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 1:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 2:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 3:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 4:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 5:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			default:
				break;
			}

			ImGui::EndChild();
		}
		ImGui::PopStyleVar();

		style->Colors[ImGuiCol_ChildBg] = ImColor(31, 24, 46);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		{
			ImGui::BeginChild("Sub Tabs", ImVec2(586.f, 25.f), false);

			ImGui::SameLine(8.f);


			if (tab == 0) {

				switch (aimbotTab) {

				case 0:
					if (ImGui::SubTab("  Rage  ", ImVec2(0.f, 25.f))) aimbotTab = 1;
					ImGui::SameLine();
					if (ImGui::SelectedSubTab("  Legit  ", ImVec2(0.f, 25.f))) aimbotTab = 0;

					ImGui::SameLine(195.f);
					switch (legitTab)
					{
					case 0:
						if (ImGui::SelectedSubTab("  Rifle  ", ImVec2(0.f, 25.f))) legitTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) legitTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) legitTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) legitTab = 3;
						break;
					case 1:
						if (ImGui::SubTab("  Rifle  ", ImVec2(0.f, 25.f))) legitTab = 0;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Sniper  ", ImVec2(0.f, 25.f))) legitTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) legitTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) legitTab = 3;
						break;
					case 2:
						if (ImGui::SubTab("  Rifle  ", ImVec2(0.f, 25.f))) legitTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) legitTab = 1;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Pistol  ", ImVec2(0.f, 25.f))) legitTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) legitTab = 3;
						break;
					case 3:
						if (ImGui::SubTab("  Rifle  ", ImVec2(0.f, 25.f))) legitTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) legitTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) legitTab = 2;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Other  ", ImVec2(0.f, 25.f))) legitTab = 3;
						break;
					default:
						break;
					}
					break;
				case 1:
					if (ImGui::SelectedSubTab("  Rage  ", ImVec2(0.f, 25.f))) aimbotTab = 1;
					ImGui::SameLine();
					if (ImGui::SubTab("  Legit  ", ImVec2(0.f, 25.f))) aimbotTab = 0;

					ImGui::SameLine(195.f);
					switch (rageTab)
					{
					case 0:
						if (ImGui::SelectedSubTab("  Auto  ", ImVec2(0.f, 25.f))) rageTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) rageTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) rageTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) rageTab = 3;
						break;
					case 1:
						if (ImGui::SubTab("  Auto  ", ImVec2(0.f, 25.f))) rageTab = 0;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Sniper  ", ImVec2(0.f, 25.f))) rageTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) rageTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) rageTab = 3;
						break;
					case 2:
						if (ImGui::SubTab("  Auto  ", ImVec2(0.f, 25.f))) rageTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) rageTab = 1;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Pistol  ", ImVec2(0.f, 25.f))) rageTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) rageTab = 3;
						break;
					case 3:
						if (ImGui::SubTab("  Auto  ", ImVec2(0.f, 25.f))) rageTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) rageTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) rageTab = 2;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Other  ", ImVec2(0.f, 25.f))) rageTab = 3;
						break;
					default:
						break;
					}
				default:
					break;
				}
			}
			else if (tab == 1)
		    {
				ImGui::SelectedSubTab("  Basic  ", ImVec2(0.f, 25.f));
			}
			else if (tab == 2)
			{
			ImGui::SelectedSubTab("  Player  ", ImVec2(0.f, 25.f));
			}
			else if (tab == 3)
			{
			ImGui::SelectedSubTab("  Misc  ", ImVec2(0.f, 25.f));
			}

			ImGui::EndChild();
		}
		ImGui::PopStyleVar();
		//ImGui::BeginColumns
		style->Colors[ImGuiCol_ChildBg] = ImColor(25, 20, 27);
		ImGui::BeginChild("Main", ImVec2(586.f, 420.f), false);
	ImGui::SameLine(7.f);
	style->Colors[ImGuiCol_ChildBg] = ImColor(21, 17, 29);
//	ImGui::BeginGroupBox("Controls", ImVec2(572, 407.f));
	switch (tab)
	{
	case 0:
		if (aimbotTab == 1)
			Ragebot();
		else
			Legitbot();
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
	//ImGui::EndGroupBox();
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
	Streamster = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Streamster_compressed_data, Streamster_compressed_size, 50.f);
	watermark = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 16.f);
	gravityBold = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 13.f);


}

